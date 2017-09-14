/**
 * @file  usb.h
 * @brief Definitions and prototypes for USB device stack
 *
 * @author Saint-Genest Gwenael <gwen@cowlab.fr>
 * @copyright Cowlab (c) 2017
 *
 * @page License
 * CowStick-bootloader is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 3 as published by the Free Software Foundation. You
 * should have received a copy of the GNU Lesser General Public
 * License along with this program, see LICENSE.md file for more details.
 * This program is distributed WITHOUT ANY WARRANTY see README file.
 */
#ifndef USB_H
#define USB_H

#define EP_BUSY       1
#define EP_ZLP        4
#define EP_ADDR    0x10
#define EP_DIR_IN  0x80
#define EP_DIR_OUT 0x00

typedef struct __attribute__((packed))
{
	/* Bank 0 */
	u32 b0_addr;
	u32 b0_pcksize;
	u16 b0_extreg;
	u8  b0_status_bk;
	u8  b0_reserved[5];
	/* Bank 1 */
	u32 b1_addr;
	u32 b1_pcksize;
	u16 b1_extreg;
	u8  b1_status_bk;
	u8  b1_reserved[5];
} ep_desc;

typedef struct
{
	int size;   /* Number of bytes to transfer        */
	int count;  /* Number of already transfered bytes */
	u32 flags;
	u8  *data;  /* Pointer to the data buffer */
} ep_status;

struct usb_module;

typedef struct usb_class
{
	void (*enable)(struct usb_module *mod);
	void (*init)  (struct usb_module *mod);
	void (*setup) (struct usb_module *mod);
	void (*sof)   (struct usb_module *mod);
	void (*xfer)  (struct usb_module *mod, u8 ep);
	/* Datas */
	void *priv;
} usb_class;

typedef struct usb_module
{
	ep_desc ep_desc[8]; /* Endpoint descriptors (see datasheet 32.8.4.1) */
	u8      ctrl[64];   /* Buffer for EP0 control */
	u8      ctrl_in[64];
	/* After here, unaligned datas */
	u8         addr;     /* Device address on bus  */
	ep_status  ep_status[8];
	u8        *desc;       /* Pointer to descriptors */
	u8        *desc_iface; /* Pointer to interface descriptors */
	usb_class *class;
} usb_module;

void usb_config   (usb_module *mod);
void usb_ep_enable(usb_module *mod, u8 ep, u8 mode);
u8  *usb_find_desc(usb_module *mod, u8 rtype, u8 type, u8 index, int *size);
void usb_init     (void);
void usb_irq      (usb_module *mod);
void usb_transfer (usb_module *mod, u8 ep, u8* data, int len);
#endif
