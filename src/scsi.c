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




EXTERNAL int usbmsc_inquiry (USBMSC *m,uint8_t *inq, size_t inq_len)
{
  uint8_t sense[24];
  uint8_t cdb[6] = { SCSI_OP_INQUIRY, 0, 0, 0, inq_len, 0 };
  int tries = 3;


  /*Incase we have just reset*/
  usbmsc_command (m, cdb, sizeof (cdb), NULL, 0, inq, inq_len, NULL, 0);

  while (usbmsc_command (m, cdb, sizeof (cdb), NULL, 0, inq, inq_len, sense,sizeof(sense))) 
    {
      usbmsc_reset (m);
      if (!(tries--))
        return -1;
    }


  /*Check the device isn't actually hiding*/
  if (((sense[0] >> 5) & 7) == 1)
    {
      info("%s: device offline",m->name);
      return -1;
    }


	if(inq)
	usbmsc_hexdump("SCSI INQUIRY: ",inq,inq_len);


return 0;
}

EXTERNAL int usbmsc_is_cdrom(USBMSC *m)
{
#if 0
uin8_t inq[128];

if (usbmsc_inquiry (m,inq,sizeof(inq)))
	return 0;

switch (inq[0] & 0x1f)
{
    case SCSI_TYPE_ROM:
	return 1;
}
#else

switch (m->type & 0x1f)
{
    case SCSI_TYPE_ROM:
	return 1;
}

#endif

return 0;
}


EXTERNAL int usbmsc_is_block(USBMSC *m)
{
#if 0
uint8_t inq[128];


if (usbmsc_inquiry (m,inq,sizeof(inq)))
	return 0;

switch (inq[0] & 0x1f)
{
    case SCSI_TYPE_ROM:
    case SCSI_TYPE_WORM:
    case SCSI_TYPE_ROM:
    case SCSI_TYPE_MOD:
	return 1;
}
#else

switch (m->type & 0x1f)
{
    case SCSI_TYPE_DISK:
    case SCSI_TYPE_WORM:
    case SCSI_TYPE_ROM:
    case SCSI_TYPE_MOD:
	return 1;
}
#endif

return 0;
}

EXTERNAL int usbmsc_read_capacity(USBMSC *m)
{
uint8_t cdb[] = { SCSI_OP_READ_CAPACITY, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t rcap[8];



  memset (rcap, 0, sizeof (rcap));

  if (usbmsc_command(m, cdb, sizeof (cdb), NULL, 0, rcap, sizeof (rcap), NULL, 0)) {
	info("%s: Read capacity failed",m->name);
	return -1;
   }

  m->num_blocks = 1ULL + ((((uint64_t) rcap[0]) << 24ULL) | (((uint64_t) rcap[1]) << 16ULL)| (((uint64_t) rcap[2]) << 8ULL)| ((uint64_t) rcap[3]));

  m->block_size = (rcap[4] << 24) | (rcap[5] << 16) | (rcap[6] << 8) | rcap[7];
  m->block_shift=floor_log2_32(m->block_size);
  m->block_mask=m->block_size-1;

  if (m->block_size != 512)
	info("%s: unusual block size %d",m->name ,m->block_size);


  m->disk_bytes = m->num_blocks * m->block_size;

	  return 0;
}




EXTERNAL int usbmsc_probe(USBMSC *m)
{
uint8_t inq[128];


if (usbmsc_inquiry (m,inq,sizeof(inq)))
	return -1;

m->type=inq[0];

  copy_and_add_null(m->vendor,inq+8,sizeof(m->vendor));
  copy_and_add_null(m->model,inq+16,sizeof(m->model));
  copy_and_add_null(m->revision,inq+32,sizeof(m->revision));
  copy_and_add_null(m->serial,inq+36,sizeof(m->serial));

if (!usbmsc_is_block(m)) return -1;
if (!usbmsc_read_capacity(m)) return -1;


return 0;
}


EXTERNAL uint64_t usbmsc_read (USBMSC *m, void *buf, uint64_t block, uint64_t nblocks)
{
  uint8_t cdb[10];

  uint64_t len = nblocks * m->block_size;

  if (nblocks > 0xffff)
	return 0;

  cdb[0] = SCSI_OP_READ_10;
  cdb[1] = 0;
  cdb[2] = (block >> 24) & 0xff;
  cdb[3] = (block >> 16) & 0xff;
  cdb[4] = (block >> 8) & 0xff;
  cdb[5] = block & 0xff;
  cdb[6] = 0;
  cdb[7] = (nblocks >> 8) & 0xff;
  cdb[8] = nblocks & 0xff;
  cdb[9] = 0;

  if (usbmsc_command (m, cdb, sizeof(cdb), NULL, 0, buf, len, NULL, 0))
    {
      warning("%s: read10 failed: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
		m->name,cdb[0],cdb[1],cdb[2],cdb[3],cdb[4],cdb[5],
		cdb[6],cdb[7],cdb[8],cdb[9]);
      return 0;
    }

  return nblocks;
}


EXTERNAL uint64_t usbmsc_write (USBMSC *m, void *buf, uint64_t block, uint64_t nblocks)
{
  uint8_t cdb[10];

  uint64_t len = nblocks * m->block_size;

  if (nblocks > 0xffff)
	return 0;

  cdb[0] = SCSI_OP_WRITE_10;
  cdb[1] = 0;
  cdb[2] = (block >> 24) & 0xff;
  cdb[3] = (block >> 16) & 0xff;
  cdb[4] = (block >> 8) & 0xff;
  cdb[5] = block & 0xff;
  cdb[6] = 0;
  cdb[7] = (nblocks >> 8) & 0xff;
  cdb[8] = nblocks & 0xff;
  cdb[9] = 0;

  if (usbmsc_command (m, cdb, sizeof(cdb), NULL, 0, buf, len, NULL, 0))
    {
      warning("%s: write10 failed: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x",
		m->name,cdb[0],cdb[1],cdb[2],cdb[3],cdb[4],cdb[5],
		cdb[6],cdb[7],cdb[8],cdb[9]);
      return 0;
    }

  return nblocks;
}

EXTERNAL int usbmsc_test_unit_ready(USBMSC *m,uint8_t *sense,int sense_len)
{
  uint8_t cdb[] = { SCSI_OP_TEST_UNIT_READY, 0, 0, 0, 0, 0 };
  int tries = 3;

if (sense_len<3) return -1;

    do 
    {
      memset (sense, 0, sense_len);
      if (!tries--)
        {
          info ("%s: not ready",m->name);
          return -1;
        }
    } while (usbmsc_command (m, cdb, sizeof (cdb), NULL, 0, NULL, 0, sense, sense_len)
	|| (sense[2] == SCSI_SK_UNIT_ATTENTION));
return 0;
}

EXTERNAL int usbmsc_start_stop(USBMSC *m,int startstop)
{
      uint8_t cdb[6] = { SCSI_OP_START_STOP, 0, 0, 0, 0, 0 };

      cdb[5]=startstop?1:0;

      info ("%s: sending %s command\n", m->name,startstop ?"start":"stop");
      if (usbmsc_command (m, cdb, sizeof (cdb), NULL, 0, NULL, 0, NULL,0)) {
          info ("%s: SCSI START_STOP command failed\n", m->name);
          return -1;
        }

  return 0;
}


EXTERNAL int usbmsc_make_ready(USBMSC *m)
{
uint8_t sense[128];

if (usbmsc_test_unit_ready(m,sense,sizeof(sense))) return -1;

if (sense[2] == SCSI_SK_NOT_READY) 
      return usbmsc_start_stop(m,1);

return 0;

}
