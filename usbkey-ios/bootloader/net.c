/**
 * @file  net.c
 * @brief Manage network interface
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
#include "net.h"
#include "net_arp.h"
#include "net_ipv4.h"
#include "libc.h"
#include "types.h"
#include "uart.h"
#include "usb_ecm.h"

static const u8 cfg_mac[6] = {0x70, 0xB3, 0xD5, 0x4C, 0xE8, 0x01};

/**
 * @brief Generic function to convert a long value to network byte order
 *
 * @param v Long integer with bytes in CPU order
 * @return  Long integer with bytes in network order
 */
u32 htonl(u32 v)
{
	u32 vout;
	vout  = ((v & 0x000000FF) << 24);
	vout |= ((v & 0x0000FF00) <<  8);
	vout |= ((v & 0x00FF0000) >>  8);
	vout |= ((v & 0xFF000000) >> 24);
	return(vout);
}

/**
 * @brief Generic function to convert a short value to network byte order
 *
 * @param v Short integer with bytes in CPU order
 * @return  Short integer with bytes in network order
 */
u16  htons(u16 v)
{
	u16 vout;
	vout = ((v & 0xFF) << 8) | (v >> 8);
	return(vout);
}

/**
 * @brief Initialize the network module
 *
 */
void net_init(network *mod)
{
	/* Clean interface structure */
	memset(mod, 0, sizeof(network));
	/* Set the default MAC address for the interface */
	memcpy(mod->mac, cfg_mac, 6);

	/* Initialize IPv4 for this interface */
	ipv4_init(mod);
}

/**
 * @brief Process network events (if any)
 *
 * This function must be called periodically to process incoming packets
 * of other periodic stack events.
 */
void net_periodic(network *mod)
{
	eth_frame *frame;

	/* If a module wait to send more datas */
	if (mod->tx_more)
	{
		frame = (eth_frame *)mod->tx_buffer;
		/* When previous packet has been sent, "proto" is set to 0 */
		if (frame->proto != 0x0000)
			return;
		/* Call the module that want to send more datas */
		mod->tx_more(mod);
		return;
	}

	/* If the network RX buffer is empty ... nothing to do */
	if (mod->rx_length == 0)
		return;

	frame = (eth_frame *)mod->rx_buffer;
	
	switch( htons(frame->proto) )
	{
		case 0x0800:
			ipv4_receive(mod, mod->rx_buffer+14, mod->rx_length-14);
			break;
		case 0x0806:
			arp_receive(mod, mod->rx_buffer+14, mod->rx_length-14);
			break;
		case 0x86DD:
			uart_puts("NET: received an IPv6 datagram\r\n");
			break;
		default:
			uart_puts("NET: data received (unknown protocol)\r\n");
	}
	ecm_rx_prepare(mod->driver);
}

/**
 * @brief Transmit a packet on the network
 *
 * @param mod  Pointer to the network interface structure
 * @param size Number of bytes to send
 */
void net_send(network *mod, u32 size)
{
	/* Call USB ECM layer to process */
	ecm_tx(mod->driver, mod->tx_buffer, size + 14);
}

/**
 * @brief Get a pointer on a buffer that can be used for TX
 *
 * @param mod   Pointer to the network interface structure
 * @param proto ID of the ethernet protocol to use into the frame
 */
u8* net_tx_buffer(network *mod, u16 proto)
{
	eth_frame *frame = 0;

	frame = (eth_frame *)mod->tx_buffer;

	if (proto != 0)
	{
		const u8 *s;
		int i;

		/* Set MAC dest (copy from received frame) */
		s = (mod->rx_buffer + 6);
		for (i = 0; i < 6; i++)
			frame->dst[i] = *s++;
		/* Set MAC source (from network interface) */
		s = mod->mac;
		for (i = 0; i < 6; i++)
			frame->src[i] = *s++;
		/* Set (ethernet) protocol */
		frame->proto = htons(proto);
	}

	return (mod->tx_buffer + 14);
}
/* EOF */
