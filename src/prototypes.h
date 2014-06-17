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

/* libusbmassstorage.c */
void usbmsc_close(USBMSC *m);
USBMSC *usbmsc_open_by_ep(libusb_device *dev, libusb_device_handle *dev_handle, int interface_number, int ep_in, int ep_out, char *name);
USBMSC *usbmsc_open_by_dev(libusb_device *dev);
/* version.c */
char *libusbmassstorage_get_version(void);
/* util.c */
uint32_t ones32(uint32_t x);
uint32_t floor_log2_32(uint32_t x);
void message(int flags, const char *file, const char *function, int line, const char *fmt, ...);
void *xmalloc(size_t s);
void *xrealloc(void *p, size_t s);
char *xstrdup(const char *s);
void copy_and_add_null(char *dst, const char *src, size_t len);
/* scsi.c */
int usbmsc_inquiry(USBMSC *m, uint8_t *inq, size_t inq_len);
int usbmsc_is_cdrom(USBMSC *m);
int usbmsc_is_block(USBMSC *m);
int usbmsc_read_capacity(USBMSC *m);
int usbmsc_probe(USBMSC *m);
uint64_t usbmsc_read(USBMSC *m, void *buf, uint64_t block, uint64_t nblocks);
uint64_t usbmsc_write(USBMSC *m, void *buf, uint64_t block, uint64_t nblocks);
int usbmsc_test_unit_ready(USBMSC *m, uint8_t *sense, int sense_len);
int usbmsc_start_stop(USBMSC *m, int startstop);
int usbmsc_make_ready(USBMSC *m);
/* transport.c */
int usbmsc_reset(USBMSC *m);
int usbmsc_command(USBMSC *m, uint8_t *cdb, size_t cdb_len, uint8_t *tx, size_t tx_len, uint8_t *rx, size_t rx_len, uint8_t *sense, size_t sense_len);
/* hexdump.c */
void usbmsc_hexdump(char *prefix, void *_d, int len);
