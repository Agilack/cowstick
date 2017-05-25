/**
 * @file  usb.h
 * @brief Definitions and prototypes for USB device stack
 *
 * @author Saint-Genest Gwenael <gwen@cowlab.fr>
 * @copyright Cowlab (c) 2017
 *
 * @page License
 * This unit-test is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation. You should have received a
 * copy of the GNU General Public License along with this program, see
 * LICENSE.md file for more details.
 * This program is distributed WITHOUT ANY WARRANTY.
 */
#ifndef USB_H
#define USB_H

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

typedef struct usb_module
{
	ep_desc ep_desc[8]; /* Endpoint descriptors (see datasheet 32.8.4.1) */
	u8      ctrl[64];   /* Buffer for EP0 control */
	u8      ctrl_in[64];
	/* After here, unaligned datas */
	u8        addr;     /* Device address on bus  */
	u8       *desc;     /* Pointer to descriptors */
	int       desc_len; /* Length of descriptors  */
	u32       flags;
} usb_module;

void usb_config(usb_module *mod);
void usb_init  (void);
void usb_irq   (usb_module *mod);

#endif