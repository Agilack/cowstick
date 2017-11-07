/**
 * @file  net_ipv4.c
 * @brief Implement IPv4 network protocol
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
#include "net_ipv4.h"
#include "libc.h"
#include "types.h"
#include "uart.h"

u8  *usbnet_rx_buffer(void);

/**
 * @brief Initialize the IPv4 protocol module
 *
 * @param mod Pointer to the network interface that will use IPv4
 */
void ipv4_init(network *mod)
{
	(void)mod;
}

/**
 * @brief Called by network layer when an IPv4 datagram is received
 *
 * @param mod    Pointer to the network interface structure
 * @param buffer Pointer to the data buffer where received datagram is stored
 * @param length Size of the received datagram (in bytes)
 */
void ipv4_receive(network *mod, u8 *buffer, int length)
{
	(void)length;
	ip_dgram *req = (ip_dgram *)buffer;

	/* Sanity check */
	if ((mod == 0) || (buffer == 0))
	{
		uart_puts("IPv4: Missing parameter for receive function\r\n");
		return;
	}

	if (req->proto == 0x01)
	{
		u8 *dgram;
		uart_puts("IPv4: receive an ICMP packet\r\n");
		dgram = ipv4_tx_buffer(mod, 0x0A0A0A03, 0x01);
		memcpy(dgram, "Hello World!\0", 13);
		ipv4_send(mod, 12);
	}
	else if (req->proto == 0x11)
	{
		uart_puts("IPv4: receive an UDP packet\r\n");
	}
	else if (req->proto == 0x06)
	{
		uart_puts("IPv4: receive a TCP packet\r\n");
	}
	else
	{
		uart_puts("IPv4:");
		uart_puts(" src="); uart_puthex( htonl(req->src) );
		uart_puts(" dst="); uart_puthex( htonl(req->dst) );
		uart_puts(" proto="); uart_puthex8( req->proto );
		uart_crlf();
	}

}

/**
 * @brief Transmit an IPv4 datagram
 *
 * @param mod Pointer to the network interface structure
 * @param len Size of the datagram to send (in bytes)
 */
void ipv4_send(network *mod, int len)
{
	ip_dgram *rsp;
	u16 cksum;

	/* Get the buffer of TX frame from network layer */
	rsp = (ip_dgram *)net_tx_buffer(mod, 0);

	/* Update datagram length */
	rsp->length = htons(20 + len);
	/* Update the IP datagram checksum */
	cksum = ip_cksum(0, (u8 *)rsp, 20);
	rsp->cksum = htons(~cksum);

	/* Call underlying net layer to send datagram */
	net_send(mod, len + 20);
}

/**
 * @brief Get a pointer on a buffer that can be used to transmit a datagram
 *
 * @param mod   Pointer to the network interface structure
 * @param dest  IP address of the remote peer
 * @param proto ID of the protocol used inside ehe datagram
 */
u8 *ipv4_tx_buffer(network *mod, u32 dest, u8 proto)
{
	ip_dgram  *rsp;

	/* Get the buffer of TX frame from network layer */
	rsp = (ip_dgram *)net_tx_buffer(mod, 0x800);
	if (rsp == 0)
		return 0;
	
	rsp->vihl   = 0x45;
	rsp->tos    = 0x00;
	rsp->length = 0x0000;
	rsp->id     = 0x0000;
	rsp->offset = 0;
	rsp->ttl    = 0x40;
	rsp->proto  = proto;
	rsp->cksum  = 0x0000;
	rsp->src    = 0xFE0A0A0A; /* equal to htonl(0x0A0A0AFE) */
	rsp->dst    = htonl(dest);
	
	return (((u8 *)rsp) + 20);
}

/**
 * @brief Compute a 16bits checksum (mainly for IPv4 header)
 *
 * @param sum  Initial value for computation
 * @param data Pointer to the data buffer
 * @param len  Length of the data buffer
 */
u16 ip_cksum(u32 sum, const u8 *data, u16 len)
{
	u16 t;
	const u8 *dataptr;
	const u8 *last_byte;

	dataptr = data;
	last_byte = data + len - 1;

	/* At least two more bytes */
	while(dataptr < last_byte)
	{	
		t = (dataptr[0] << 8) + dataptr[1];
		sum += t;
//		if(sum < t)
//			sum++; /* carry */
		dataptr += 2;
	}
	
	if (len & 1)
	{
		t = (dataptr[0] << 8) + 0;
		sum += t;
//		if(sum < t)
//			sum++; /* carry */
      }

	while (sum & 0xffff0000)
	{
		u16 plop;
		plop = (sum >> 16);
		sum = (sum & 0xFFFF) + plop;
	}
	
	/* Return sum in host byte order. */
	return (u16)sum;
}
/* EOF */
