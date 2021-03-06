/**
 * @file  usb_desc.h
 * @brief USB Descriptors for ethernet control model
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
#ifndef USB_DESC_H
#define USB_DESC_H

const unsigned char usb_ecm_desc[] = {
	/* ---- Device Descriptor ---- */
	18,   0x01, 0x02, 0x00, 0xEF, 0x02, 0x01, 0x40,
	0xeb, 0x03, 0x21, 0x24, 0x00, 0x01, 0x00, 0x01,
	0x02, 0x01, 
	/* ---- Lang Descriptor ---- */
	0x04, 0x03, 0x09, 0x04,
	/* String #1 Product */
	18, 0x03,
	'C',0x00, 'o',0x00, 'w',0x00, 's',0x00,
	't',0x00, 'i',0x00, 'c',0x00, 'k',0x00,
	/* String #2 MAC Address */
	26, 0x03,
	'7',0x00, '0',0x00, 'B',0x00, '3',0x00,
	'D',0x00, '5',0x00, '4',0x00, 'C',0x00,
	'E',0x00, '8',0x00, '0',0x00, '0',0x00,
	/* ---- Configuration Descriptor ----*/
	0x09, 0x02,  71 , 0x00, 0x02, 0x01, 0x00, 0x80,
	0x32,
	/* ---- Interface Descriptor ---- */
	0x09, 0x04, 0x00, 0x00, 0x01, 0x02, 0x06, 0x00,
	0x00,
	/* ---- Header Functional Descriptor ----*/
	0x05, 0x24, 0x00, 0x20, 0x01,
	/* ---- Functional Descriptor ---- */
	0x05, 0x24, 0x06, 0x00, 0x01,
	/* ---- Functional Descriptor */
	13,   0x24, 0x0F, 0x02, 0x00, 0x00, 0x00, 0x00,
	0xEA, 0x05, 0x00, 0x00, 0x00,
	/* ---- Endpoint ---- */
	0x07, 0x05, 0x83, 0x03, 0x40, 0x00, 0xFF,
	/* ---- Interface Descriptor ---- */
	0x09, 0x04, 0x01, 0x00, 0x02, 0x0A, 0x00, 0x00,
	0x00,
	/* ---- Endpoint ---- */
	0x07, 0x05, 0x01, 0x02, 0x40, 0x00, 0x00,
	/* ---- Endpoint ---- */
	0x07, 0x05, 0x82, 0x02, 0x40, 0x00, 0x00,

	/* Empty descriptor (End Of Descriptors marker) */
	0x00, 0x00
};

#endif
