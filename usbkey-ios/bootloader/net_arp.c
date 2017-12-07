/**
 * @file  net_arp.c
 * @brief This module handle ARP network protocol
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
#include "log.h"
#include "net.h"
#include "net_arp.h"
#include "libc.h"

/**
 * @brief Called when an ARP packet is received
 *
 * @param mod    Pointer to the network interface structure
 * @param buffer Pointer to the received data buffer
 * @param length Length of the received packet (in bytes)
 */
void arp_receive(network *mod, u8 *buffer, int length)
{
	arp_packet *req = (arp_packet *)buffer;
	
	/* Sanity check */
	if ((buffer == 0) || (length == 0)) {
		ARP_PUTS("arp_receive() Bad buffer\r\n");
		return;
	}

	/* Test hardware type (only Ethernet is supported) */
	if (htons(req->type)  != 0x0001) {
		ARP_PUTS("arp_receive() Bad hardware type\r\n");
		return;
	}
	/* Test protocol type (only IPv4 is supported) */
	if (htons(req->proto) != 0x0800) {
		ARP_PUTS("arp_receive() Bad protocol\r\n");
		return;
	}

	if (htons(req->op) == 0x0001)
	{
		/* Is the requested IP is me ? */
		if (htonl(req->dst_ip) == CFG_IP_LOCAL)
		{
			arp_packet *rsp;
			
			rsp = (arp_packet *)net_tx_buffer(mod, 0x806);
			rsp->type  = 0x0100; /* equal to htons(0x0001) */
			rsp->proto = req->proto;
			rsp->hlen  = 0x06;
			rsp->llen  = 0x04;
			rsp->op    = 0x0200; /* equal to htons(0x0002) */
			memcpy(rsp->src_phy, mod->mac, 6);
			rsp->src_ip= htonl(CFG_IP_LOCAL);
			memcpy(rsp->dst_phy, req->src_phy, 6);
			rsp->dst_ip= req->src_ip;
			
#ifdef DEBUG_ARP
			DBG_PUTS("NET_ARP: Resquest from ");
			DBG_PUTHEX8(req->src_phy[0]); DBG_PUTC(':');
			DBG_PUTHEX8(req->src_phy[1]); DBG_PUTC(':');
			DBG_PUTHEX8(req->src_phy[2]); DBG_PUTC(':');
			DBG_PUTHEX8(req->src_phy[3]); DBG_PUTC(':');
			DBG_PUTHEX8(req->src_phy[4]); DBG_PUTC(':');
			DBG_PUTHEX8(req->src_phy[5]);
			DBG_PUTS("\r\n");
#endif

			/* Send response */
			net_send(mod, sizeof(arp_packet));
		}
	}
	else
	{
		/* Packet is not a request */
	}
}
/* EOF */
