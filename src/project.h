/*
 * project.h:
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


/*
 * $Id:$
 */

/*
 * $Log:$
 *
 */

#ifndef __PROJECT_H__
#define __PROJECT_H__

#define _GNU_SOURCE

#include "config.h"

#ifdef TM_IN_SYS_TIME
#include <sys/time.h>
#ifdef TIME_WITH_SYS_TIME
#include <time.h>
#endif
#else
#ifdef TIME_WITH_SYS_TIME
#include <sys/time.h>
#endif
#include <time.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#if defined(HAVE_STDINT_H)
#include <stdint.h>
#elif defined(HAVE_SYS_INT_TYPES_H)
#include <sys/int_types.h>
#endif

#ifdef INT_PROTOS
#define INTERNAL
#define EXTERNAL
#else 
#ifdef EXT_PROTOS
#define INTERNAL static
#define EXTERNAL
#else
#define INTERNAL
#define EXTERNAL
#endif
#endif


#include <stdarg.h>

#include "usbmassstorage.h"

#include <libusb.h>

struct usbmsc_struct {
	/*libusb stuffs*/
        libusb_device *dev;
        libusb_device_handle *dev_handle;
	int dev_handle_is_ours;

	/*name for error messages*/
	char name[128];

        /*Usb stuffs*/
        int interface_number;

        int ep_in,ep_out;

        /*scsi stuffs*/
 	int type;

	char vendor[9];
	char model[17];
	char revision[5];
	char serial[21];

        /*block device stuffs*/
        uint64_t num_blocks;
        uint32_t block_size;
        uint64_t block_shift;
        uint64_t block_mask;
        uint64_t disk_bytes;

};

#include "util.h"
#include "prototypes.h"

#endif /* __PROJECT_H__ */




