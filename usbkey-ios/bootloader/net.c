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
#include "libc.h"
#include "types.h"
#include "uart.h"
#include "usb_ecm.h"

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
	
	/* Limit buffer size before dump on console - DEBUG ONLY */
	if (mod->rx_length > 0x50)
		mod->rx_length = 0x50;

	switch( htons(frame->proto) )
	{
		case 0x0800:
			uart_puts("NET: received an IPv4 datagram\r\n");
			uart_dump((u8 *)frame, mod->rx_length);
			break;
		case 0x0806:
			uart_puts("NET: received a MAC packet\r\n");
			uart_dump((u8 *)frame, mod->rx_length);
			break;
		case 0x86DD:
			uart_puts("NET: received an IPv6 datagram\r\n");
			break;
		default:
			uart_puts("NET: data received (unknown protocol)\r\n");
	}
	ecm_rx_prepare(mod->driver);
}

/* EOF */
