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
#include "net.h"

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
}

/**
 * @brief Prepare an RX buffer for USB transfer
 *
 * @param mod Pointer to the USB module
 */
void ecm_rx_prepare(usb_module *mod)
{
	network *net;

	/* Sanity check : a network interface must been attached to the ECM */
	if ((mod->class == 0) || (mod->class->priv == 0))
	{
		uart_puts("esb_ecm: RX prepare fails, no network intferface\r\n");
		return;
	}

	/* Get network interface from USB class private data */
	net = (network *)mod->class->priv;

	/* Sanity check : interface must have an RX buffer */
	if (net->rx_buffer == 0)
	{
		uart_puts("usb_ecm: RX prepare fails, no buffer\r\n");
		return;
	}

	/* Set buffer length to 0 */
	net->rx_length = 0;
	/* Start (prepare) a transfer on endpoint */
	usb_transfer(mod, 1, net->rx_buffer, 512);
}

/**
 * @brief Send a network packet over USB ECM
 *
 * @param mod    Pointer to network interface structure
 * @param buffer Pointer to the data buffer to send
 * @param size   Size of the packet (in bytes)
 */
void ecm_tx(usb_module *mod, u8 *buffer, u32 size)
{
	usb_transfer(mod, 0x82, buffer, size);
}

/**
 * @brief Called by USB stack when the device is enabled
 *
 * The device is enabled when a configuration is selected by the remote host
 * (using SET CONFIGURATION). 
 *
 * @param mod Pointer to the USB module configuration
 */
void cb_enable(usb_module *mod)
{
	network *net;
	u8      *rx_buffer;

	/* Enable endpoint 1 for datas host -> device (bulk OUT) */
	usb_ep_enable(mod, 1, 0x03);
	/* Enable endpoint 2 for datas device -> host (bulk IN) */
	usb_ep_enable(mod, 2, 0x30);
	/* Enable endpoint 3 for CDC control (interrupt IN) */
	usb_ep_enable(mod, 3, 0x40);

	/* Sanity check : a network interface must been attached to the ECM */
	if ((mod->class == 0) || (mod->class->priv == 0))
	{
		uart_puts("esb_ecm: Enable error, no network intferface\r\n");
		return;
	}

	/* Get network interface from USB class private data */
	net = (network *)mod->class->priv;
	/* Get RX buffer (if any) */
	rx_buffer = net->rx_buffer;

	if (rx_buffer)
		usb_transfer(mod, 1, rx_buffer, 512);
	else
		uart_puts("usb_ecm: Enable error, no RX buffer\r\n");
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

/**
 * @brief Called by USB layer when a transfer is complete on ECM endpoint
 *
 * @param mod Pointer to the USB module
 * @param ep  Endpoint id
 */
static void cb_xfer(usb_module *mod, u8 ep)
{
	network *net;

	if ((mod->class == 0) || (mod->class->priv == 0))
	{
		uart_puts("usb_ecn: ERROR, no network interface\r\n");
		return;
	}

	/* Get network interface from USB class private data */
	net = (network *)mod->class->priv;

	switch (ep)
	{
		/* RX */
		case 0x01:
			/* Update buffer length wth count of received datas */
			net->rx_length = mod->ep_status[ep].count;
			break;
		/* TX */
		case 0x02:
			uart_puts("usb_ecm: TX complete\r\n");
			/* Clear ethernet header */
			memset(net->tx_buffer, 0, 14);
	}
}
/* EOF */
