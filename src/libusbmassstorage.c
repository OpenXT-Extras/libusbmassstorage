/*
 * libusbmassstorage.c:
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
 *
 */

#include "project.h"

EXTERNAL void usbmsc_close(USBMSC *m)
{

if(m &&m->dev_handle_is_ours) 
	libusb_close(m->dev_handle);

free(m);
}

EXTERNAL USBMSC *usbmsc_open_by_ep(libusb_device *dev, libusb_device_handle *dev_handle,
	int interface_number,int ep_in,int ep_out,char * name)
{

USBMSC *ret;


ret=xmalloc(sizeof(*ret));

strncpy(ret->name,name,sizeof(ret->name));
ret->name[sizeof(ret->name)-1]=0;


ret->dev=dev;
ret->dev_handle=dev_handle;
ret->interface_number=interface_number;
ret->ep_in=ep_in;
ret->ep_out=ep_out;

ret->dev_handle_is_ours=0;


return ret;

}


EXTERNAL USBMSC *usbmsc_open_by_dev(libusb_device *dev)
{

//ret->dev_handle_is_ours=1;

// XXX: fill me in
 
return NULL;

}


