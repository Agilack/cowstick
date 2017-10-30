/**
 * @file  usb_ecm.c
 * @brief USB class driver for Ethernet Control Model (ECM)
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
#include "libc.h"
#include "types.h"
#include "usb.h"
#include "uart.h"

static void cb_enable(usb_module *mod);
static void cb_setup (usb_module *mod);
static void cb_xfer  (usb_module *mod, u8 ep);

/**
 * @brief Initialize 
 *
 * @param mod Pointer to the USB module configuration
 */
void ecm_init(usb_module *mod, usb_class *obj)
{
	/* Clean memory */
	memset(obj, 0, sizeof(usb_class));
	/* Configure ECM callback functions */
	obj->enable = cb_enable;
	obj->setup  = cb_setup;
	obj->xfer   = cb_xfer;
	/* Register the class into USB module */
	mod->class = obj;
	
	uart_puts("ecm_init\r\n");
}

/**
 * @brief Called by USB stack when the device is enabled
 *
 * The device is enabled when a configuration is selected by the remote host
 * (using SET CONFIGURATION). 
 *
 * @param mod Pointer to the USB module configuration
 */

u8 ep1[512];

void cb_enable(usb_module *mod)
{
	/* Enable endpoint 1 for datas host -> device (bulk OUT) */
	usb_ep_enable(mod, 1, 0x03);
	/* Enable endpoint 2 for datas device -> host (bulk IN) */
	usb_ep_enable(mod, 2, 0x30);
	/* Enable endpoint 3 for CDC control (interrupt IN) */
	usb_ep_enable(mod, 3, 0x40);
	
	usb_transfer(mod, 1, ep1, 512);
}

/**
 * @brief Called by USB stack when a request for "class" is received on EP0
 *
 * @param mod Pointer to the USB module configuration
 */
static void cb_setup (usb_module *mod)
{
	u8 bmRequestType = (mod->ctrl[0] & 0x1F);

	/* In case of an Interface request */
	if (bmRequestType == 0x01)
	{
		/* SET_ETHERNET_PACKET_FILTER */
		if (mod->ctrl[1] == 0x43)
			/* Send ZLP for Status phase  */
			usb_transfer(mod, 0x80, 0, 0);
		/* USB_REQ_CDC_SET_LINE_CODING */
		else if (mod->ctrl[1] == 0x20)
			usb_transfer(mod, 0x80, 0, 0);
		/* USB_REQ_CDC_SET_CONTROL_LINE_STATE */
		else if (mod->ctrl[1] == 0x22)
			usb_transfer(mod, 0x80, 0, 0);
	}
}

static void cb_xfer(usb_module *mod, u8 ep)
{
	uart_puts("ECM: xfer\r\n");
	uart_dump(ep1, mod->ep_status[ep].count);

	usb_transfer(mod, 1, ep1, 512);
}
