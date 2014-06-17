/*
 * Copyright (c) 2012 Citrix Systems, Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "project.h"
#include "scsireg.h"

#define TIMEOUT 5000


struct __attribute__ ((packed)) cbw
{
  uint32_t Signature;
  uint32_t Tag;
  uint32_t DataTransferLength;
  uint8_t Flags;
  uint8_t Lun;
  uint8_t Length;
  uint8_t CDB[16];
};

#define CBW_SIGNATURE	0x43425355 //'USBC'


struct __attribute__ ((packed)) cbs
{
  uint32_t Signature;           /* should = 'USBS' */
  uint32_t Tag;                 /* same as original command */
  uint32_t Residue;             /* amount not transferred */
  uint8_t Status;               /* see below */
};

#define CBS_SIGNATURE    	0x53425355 // 'USBS'
#define CBS_SV_CMD_PASSED	0
#define CBS_SV_CMD_FAILED	1
#define CBS_SV_PHASE_ERROR      2





EXTERNAL int
usbmsc_reset (USBMSC * m)
{
  int ret = 0, i;

  info("reset");

/* USB reset procedure: section 5.4 of the usb mass storage class bulk-only
 * transport spec rev 1.0 */

  i = libusb_control_transfer (m->dev_handle,
                               LIBUSB_REQUEST_TYPE_CLASS |
                               LIBUSB_RECIPIENT_INTERFACE, 0xff, 0,
                               m->interface_number, NULL, 0, TIMEOUT);

  if (i < 0)
    ret = i;

  i = libusb_clear_halt (m->dev_handle, m->ep_in);
  if (i < 0)
    ret = i;

  i = libusb_clear_halt (m->dev_handle, m->ep_out);
  if (i < 0)
    ret = i;


  return ret;
}


static int
usbmsc_command_worker (USBMSC * m,
                   uint8_t * cdb, size_t cdb_len,
                   uint8_t * tx, size_t tx_len,
                   uint8_t * rx, size_t rx_len)
//                   uint8_t * sense, size_t sense_len)
{
  struct cbw cbw;
  struct cbs cbs;
  static uint32_t serial = 12;
  int bytes_shipped;
  int res;
  int tries;

/*Command phase*/

  cbw.Signature = CBW_SIGNATURE;
  cbw.DataTransferLength = rx ? rx_len : tx_len;
  cbw.Flags = rx ? (1 << 7) : 0;
  cbw.Tag = serial++;
  cbw.Lun = cdb[1] >> 5;
  cbw.Length = cdb_len;

		usbmsc_hexdump("cdb: ",cdb,cdb_len);

  if (cdb_len > sizeof (cbw.CDB))
    return -1;
  memset (cbw.CDB, 0, sizeof (cbw.CDB));
  memcpy (cbw.CDB, cdb, cdb_len);

  res = libusb_bulk_transfer (m->dev_handle,
                              m->ep_out, (char *) &cbw, sizeof (cbw),
                              &bytes_shipped, TIMEOUT);

  info("cmd phase 1 cbw %d/%d %d\n",bytes_shipped,sizeof(cbw),res);
		usbmsc_hexdump("cbw: ",&cbw,sizeof(cbw));

  if ((res) || (bytes_shipped != sizeof (cbw)))
    {
      usbmsc_reset (m);
      return res ? res : -1;
    }


/*Data phase*/
  if (rx)
    {
      /*In */
      res = libusb_bulk_transfer (m->dev_handle, m->ep_in, (char *)
                                  rx, rx_len, &bytes_shipped, TIMEOUT);

  info("cmd phase 2  rx %d/%d %d\n",bytes_shipped,rx_len,res);
      if ((res==LIBUSB_ERROR_PIPE) || (bytes_shipped != rx_len)){
	/* endpoint has stalled, we carry on regardless because the usw tells us how much
         * data to trust, however we must unstall the endpoint */
	info("clear in halt");
        libusb_clear_halt (m->dev_handle, m->ep_in);
       } 
      else if (res) {
          usbmsc_reset (m);
          return res ? res : -1;
        }

	if (bytes_shipped)
		usbmsc_hexdump("rx: ",rx,bytes_shipped);

    }
  else
    {
      /*Out */
      res = libusb_bulk_transfer (m->dev_handle, m->ep_out, (char *)
                                  tx, tx_len, &bytes_shipped, TIMEOUT);
  info("cmd phase 2  tx %d/%d %d\n",bytes_shipped,tx_len,res);

      if ((res==LIBUSB_ERROR_PIPE) || (bytes_shipped != tx_len)){
	/* endpoint has stalled, we carry on regardless because the usw tells us how much
         * data to trust, however we must unstall the endpoint */
        libusb_clear_halt (m->dev_handle, m->ep_out);
	info("clear out halt");
       } else  if (res)
        {
          usbmsc_reset (m);
          return res ? res : -1;
        }

    }

/* Status phase*/


  tries = 3;
  do
    {
      res = libusb_bulk_transfer (m->dev_handle, m->ep_in, (char *)
                                  &cbs, sizeof (cbs), &bytes_shipped,
                                  TIMEOUT);
  info("cmd phase 3 cbs %d/%d %d\n",bytes_shipped,sizeof(cbs),res);
      if (res) {
	info("clear in halt");
        libusb_clear_halt (m->dev_handle, m->ep_in);
	}
    }
  while ((tries--) && res);

  if ((res) || (bytes_shipped != sizeof (cbs)))
    {
      usbmsc_reset (m);
      return -1;
    }

  if (cbs.Signature!=CBS_SIGNATURE) 
    {
      usbmsc_reset (m);
      return -1;
    }

//FIXME check length here
//
  switch (cbs.Status)
    {
    case CBS_SV_CMD_FAILED:
    case CBS_SV_PHASE_ERROR:
    default:
      usbmsc_reset (m);
      return -1;
    case CBS_SV_CMD_PASSED:
	break;
    }

  return 0;

}



static int
usbmsc_sense_worker (USBMSC * m, uint8_t * sense, size_t sense_len)
{
  uint8_t sense_buf[128];
  uint8_t request_sense_cdb[6] = { SCSI_OP_REQUEST_SENSE, 0, 0, 0, 0, 0 };


  info("getsense");

  if (!sense)
    return;

  if (!sense)
    {
      sense = sense_buf;
      sense_len = sizeof (sense_buf);
    }


  request_sense_cdb[4] = sense_len;

  return usbmsc_command_worker (m, request_sense_cdb, sizeof (request_sense_cdb),
                            NULL, 0, sense, sense_len);
}

EXTERNAL int
usbmsc_command (USBMSC * m,
            uint8_t * cdb, size_t cdb_len,
            uint8_t * tx, size_t tx_len,
            uint8_t * rx, size_t rx_len,
            uint8_t * sense, size_t sense_len)
{
  int tries = 3;
  int ret;

  do
    {
      ret = usbmsc_command_worker (m, cdb, cdb_len, tx, tx_len, rx, rx_len);

      if (ret)
        continue;

      /* Get sense */

      ret = usbmsc_sense_worker (m, sense, sense_len);

    }
  while ((tries--) && ret);


  return ret;
}
