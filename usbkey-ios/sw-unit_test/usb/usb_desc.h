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
	0x07, 0x05, 0x81, 0x03, 0x08, 0x00, 0x0A,
	/* HID Report Descriptor */
	0x05, 0x01, /* Usage Page (Generic Desktop) */
	0x09, 0x02, /* Usage (Mouse) */
	0xA1, 0x01, /* Collection (Application) */
	0x09, 0x01, /* Usage (Pointer) */
	0xA1, 0x00, /* Collection (Physical) */
	0x05, 0x09, /* Usage Page (Buttons) */
	0x19, 0x01, /* Usage Minimum (01) */
	0x29, 0x03, /* Usage Maximum (03) */
	0x15, 0x00, /* Logical Minimum (0) */
	0x95, 0x03, /* Report Count (3) */
	0x75, 0x01, /* Report Size (1) */
	0x25, 0x01, /* Logical Maximum (1) */
	0x81, 0x02, /* Input (Data, Variable, Absolute) */
	0x95, 0x01, /* Report Count (1) */
	0x75, 0x05, /* Report Size (5) */
	0x81, 0x01, /* Input (Constant) ;5 bit padding */
	0x05, 0x01, /* Usage Page (Generic Desktop) */
	0x09, 0x30, /* Usage (X) */
	0x09, 0x31, /* Usage (Y) */
	0x16, 0x71, 0x00, /* USAGE_MINIMUM */
	0x26, 0x7F, 0x00, /* USAGE_MAXIMUM */
	0x75, 0x10, /* Report Size (16) */
	0x95, 0x02, /* Report Count (2) */
	0x81, 0x02, /* Input (Data, Variable, Absolute) */
	0xC0, 0xC0  /* End Collection,End Collection */
};

#endif
