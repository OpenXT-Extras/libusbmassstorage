/*
 * usbmassstorage.c:
 *
 *
 */

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


static char rcsid[] = "$Id:$";

/*
 * $Log:$
 */

#include "usbmassstorage.h"

static libusb_context *ud_usb_ctx = NULL;

void sniff_harder(libusb_device *dev, int ifn, const struct libusb_interface_descriptor *id)
{
struct libusb_device_handle *devh;
USBMSC *m;

int epn;
int iep,oep;

iep=oep=-1;



for (epn = 0; epn < id->bNumEndpoints; ++epn) {

if ((id->endpoint[epn].bmAttributes & LIBUSB_TRANSFER_TYPE_MASK)!= LIBUSB_TRANSFER_TYPE_BULK)
		continue;

if ((id->endpoint[epn].bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_IN) iep=id->endpoint[epn].bEndpointAddress;
if ((id->endpoint[epn].bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_OUT) oep=id->endpoint[epn].bEndpointAddress;

}

if (iep==-1) return;
if (oep==-1) return;


devh=NULL;
libusb_open(dev,&devh);

if (!devh) return;


libusb_detach_kernel_driver(devh,ifn);

libusb_claim_interface(devh,ifn);


m=usbmsc_open_by_ep(dev,devh,ifn,iep,oep,"fish");

{
char  buf[30];

usbmsc_reset(m);
usbmsc_inquiry (m,buf,sizeof(buf));
}



usbmsc_close(m);

libusb_close(devh);
}
	
void sniff(libusb_device *dev)
{
struct libusb_device_descriptor dev_descriptor;
struct libusb_config_descriptor *config_descriptor;
const struct libusb_interface *iface;
const struct libusb_interface_descriptor *id;
int cfg,ifn;


libusb_get_device_descriptor (dev, &dev_descriptor);


for ( cfg=0; cfg<dev_descriptor.bNumConfigurations;++cfg)
{

libusb_get_config_descriptor (dev, cfg, &config_descriptor);

for (ifn=0;ifn<config_descriptor->bNumInterfaces;++ifn) {
iface=&config_descriptor->interface[ifn];

/*We don't care about alternates*/
id=&iface->altsetting[0];

if (id->bInterfaceClass==8) {


if (((id->bInterfaceSubClass==6) && (id->bInterfaceProtocol==80)) 
  || ((id->bInterfaceSubClass == 1 ) && (id->bInterfaceProtocol == 1 ))) {

		


	sniff_harder(dev,ifn,id);
	

}
}
}


libusb_free_config_descriptor(config_descriptor);




}
}

int main(int argc,char *argv[])
{
  int n, i;
  libusb_device **devs;

  libusb_init (&ud_usb_ctx);

  n = libusb_get_device_list (ud_usb_ctx, &devs);

  for (i = 0; i < n; ++i)
    {
	      libusb_ref_device (devs[i]);

		sniff(devs[i]);
		libusb_unref_device (devs[i]);

   }


  libusb_free_device_list (devs, 1);
  libusb_exit (ud_usb_ctx);

return 0;
}
