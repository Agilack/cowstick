/**
 * @file  usb_desc.h
 * @brief Descriptor for USB mouse device
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
#ifndef USB_DESC_H
#define USB_DESC_H

unsigned char mouse_desc_array[] = {
	/* Device Descriptor */
	18,   0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x40,
	0xeb, 0x03, 0x00, 0x24,	0x00, 0x01, 0x00, 0x00,
	0x00, 0x01,
	/* Language Descriptor */
	0x04, 0x03, 0x09, 0x04,
	/* Configuration Descriptor */
	0x09, 0x02, 34, 0x00, 0x01, 0x01, 0x00, 0x80,
	0x32,
	/* Interface Descriptor */
	0x09, 0x04, 0x00, 0x00, 0x01, 0x03, 0x01, 0x02,
	0x00,
	/* Functional Descriptor */
	0x09, 0x21, 0x10, 0x01, 0, 0x01, 34, 0x34, 0x00,
	/* Endpoint Descriptor */
	0x07, 0x05, 0x81, 0x03, 0x08, 0x00, 0x0A
};

#endif
