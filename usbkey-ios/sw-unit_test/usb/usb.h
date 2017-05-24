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

void usb_config(void);
void usb_init(void);
void usb_irq(void);

#endif
