/**
 * @file  net_dhcp.c
 * @brief Implement DHCP network service
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
#include "net.h"
#include "net_dhcp.h"
#include "net_ipv4.h"
#include "uart.h"

/**
 * @brief Called by UDP layer when a packet is received on DHCP port
 *
 * @param netif Pointer to the network interface structure
 * @param udp   Pointer to the UDP packet structure
 * @param ip    Pointer to the received IP datagram
 */
void dhcp_recv(network *netif, udp_packet *udp, ip_dgram *ip)
{
	udp_conn     conn;
	dhcp_packet *pkt = (dhcp_packet *)( ((u8 *)udp) + 8);
	
	(void)ip; /* To avoid compiler warning */

	uart_puts("DHCP:");
	uart_puts(" op="); uart_puthex8(pkt->op);
	uart_crlf();
	
	if (pkt->op == DHCP_DISCOVER)
	{
		/* Initialize a temporary UDP connection */
		conn.ip_remote   = 0xFFFFFFFF;
		conn.port_remote = udp->src_port;
		conn.port_local  = htons(0x43);
		conn.rsp = 0;
		/* Make DHCP response */
		dhcp_packet *dhcp = (dhcp_packet *)udp4_tx_buffer(netif, &conn);
		dhcp->op     = DHCP_OFFER;
		dhcp->htype  = 1;
		dhcp->hlen   = 6;
		dhcp->xid    = pkt->xid;
		dhcp->secs   = 0;
		dhcp->flags  = 0;
		dhcp->ciaddr = 0x00000000;
		dhcp->yiaddr = htonl(0x0A0A0A03);
		dhcp->siaddr = htonl(0x0A0A0AFE);
		dhcp->giaddr = 0x00000000;
		memset(dhcp->chaddr, 0,  16);
		memcpy(dhcp->chaddr, pkt->chaddr, dhcp->hlen);
		memset(dhcp->sname,  0,  64);
		memset(dhcp->file,   0, 128);
		/* Send response into a UDP packet */
		udp4_send(netif, &conn, sizeof(dhcp_packet) );
	}
}
/* EOF */
