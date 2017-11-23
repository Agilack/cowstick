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

static void dhcpd_discover(network *netif, udp_packet *udp, int is_dhcp);
static void dhcpd_request (network *netif, udp_packet *udp);
static u8*  find_option(dhcp_packet *pkt, u8 id);

/**
 * @brief Called by UDP layer when a packet is received on DHCP port
 *
 * @param netif Pointer to the network interface structure
 * @param udp   Pointer to the UDP packet structure
 * @param ip    Pointer to the received IP datagram
 */
void dhcp_recv(network *netif, udp_packet *udp, ip_dgram *ip)
{
	dhcp_packet *pkt = (dhcp_packet *)( ((u8 *)udp) + 8);
	
	(void)ip; /* To avoid compiler warning */

	if (pkt->op == DHCP_DISCOVER)
	{
		/* Test if packet length can contain DHCP fields */
		if (htons(udp->length) > (sizeof(dhcp_packet) + 4))
		{
			u8 *options = (u8 *)pkt + sizeof(dhcp_packet);
			/* Get the first BOOTP vendor extension */
			u32 cookie = (options[0] << 24) | (options[1] << 16) |
			             (options[2] <<  8) | (options[3]);
			/* If the extension contains DHCP magic cookie */
			if (cookie == 0x63825363)
			{
				u8 *ptype = find_option(pkt, 53);
				u8 type;
				/* If the DHCP message type is not found ... */
				if (ptype == 0)
				{
					/* revert to BOOTP mode */
					dhcpd_discover(netif, udp, 0);
					return;
				}

				type = ptype[2];
				/* In case of a DHCP DISCOVER */
				if (type == 1)
					dhcpd_discover(netif, udp, 1);
				/* In case of a DHCP REQUEST */
				else if (type == 3)
					dhcpd_request(netif, udp);
			}
			/* Failed to find magic cookie, use BOOTP mode */
			else
				dhcpd_discover(netif, udp, 0);
		}
		/* No :( Use the BOOTP mode */
		else
			dhcpd_discover(netif, udp, 0);
	}
}

/**
 * @brief Process a DHCP discover (or a BOOTP request)
 *
 * @param netif   Pointer to the network interface structure
 * @param udp     Pointer to the UDP packet of the request
 * @param is_dhcp Boolean, true if DHCP request, false if BOOTP request
 */
static void dhcpd_discover(network *netif, udp_packet *udp, int is_dhcp)
{
	udp_conn     conn;
	int size;

	dhcp_packet *pkt = (dhcp_packet *)( ((u8 *)udp) + 8);

	DHCP_PUTS("DHCP DISCOVER\r\n");

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
	dhcp->yiaddr = htonl(CFG_IP_REMOTE);
	dhcp->siaddr = htonl(CFG_IP_LOCAL);
	dhcp->giaddr = 0x00000000;
	memset(dhcp->chaddr, 0,  16);
	memcpy(dhcp->chaddr, pkt->chaddr, dhcp->hlen);
	memset(dhcp->sname,  0,  64);
	memset(dhcp->file,   0, 128);
	size = sizeof(dhcp_packet);

	if (is_dhcp)
	{
		u8 *options;
		options = ((u8 *)dhcp) + sizeof(dhcp_packet);
		/* Insert DHCP magic cookie */
		options[0] = 0x63;
		options[1] = 0x82;
		options[2] = 0x53;
		options[3] = 0x63;
		options += 4;
		/* DHCP message type */
		options[0] = 53;
		options[1] = 1;
		options[2] = 2; /* OFFER */
		options += 3;
		/* Lease time */
		options[0] = 51;
		options[1] = 4;
		options[2] = 0;  /* 4h */
		options[3] = 0;
		options[4] = 0x38;
		options[5] = 0x40;
		options += 6;
		/* Renewal time */
		options[0] = 58;
		options[1] = 4;
		options[2] = 0; /* 2h */
		options[3] = 0;
		options[4] = 0x1C;
		options[5] = 0x20;
		options += 6;
		/* End of options */
		options[0] = 0xFF;
		/* Update packet size */
		size += 4 + 3 + 6 + 6 + 1;
	}
	/* Send response into a UDP packet */
	udp4_send(netif, &conn, size);
}

/**
 * @brief Process a DHCP request
 *
 * @param netif   Pointer to the network interface structure
 * @param udp     Pointer to the UDP packet of the request
 */
static void dhcpd_request(network *netif, udp_packet *udp)
{
	udp_conn     conn;
	u8 *options;

	dhcp_packet *pkt = (dhcp_packet *)( ((u8 *)udp) + 8);

	DHCP_PUTS("DHCP REQUEST\r\n");

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
	dhcp->yiaddr = htonl(CFG_IP_REMOTE);
	dhcp->siaddr = htonl(CFG_IP_LOCAL);
	dhcp->giaddr = 0x00000000;
	memset(dhcp->chaddr, 0,  16);
	memcpy(dhcp->chaddr, pkt->chaddr, dhcp->hlen);
	memset(dhcp->sname,  0,  64);
	memset(dhcp->file,   0, 128);
	/* Include DHCP options */
	options = ((u8 *)dhcp) + sizeof(dhcp_packet);
	options[0] = 0x63;
	options[1] = 0x82;
	options[2] = 0x53;
	options[3] = 0x63;
	options += 4;
	/* DHCP message type */
	options[0] = 53;
	options[1] = 1;
	options[2] = 5; /* ACK */
	options += 3;
	/* Lease time */
	options[0] = 51;
	options[1] = 4;
	options[2] = 0; /* 4h */
	options[3] = 0;
	options[4] = 0x38;
	options[5] = 0x40;
	options += 6;
	/* Renewal time */
	options[0] = 58;
	options[1] = 4;
	options[2] = 0; /* 2h */
	options[3] = 0;
	options[4] = 0x1C;
	options[5] = 0x20;
	options += 6;
	/* End of options */
	options[0] = 0xFF;
	/* Send response into a UDP packet */
	udp4_send(netif, &conn, sizeof(dhcp_packet)+20 );
}

/**
 * @brief Search a specific DHCP option into a packet
 *
 * @param pkt Pointer to the DHCP packet where to search
 * @param id  Identifier of the searched option
 * @return Pointer to the extension (or NULL if not found)
 */
static u8* find_option(dhcp_packet *pkt, u8 id)
{
	u8 *pnt = (u8 *)pkt + sizeof(dhcp_packet) + 4;
	u8 *result = 0;

	while(1)
	{
		/* If the current option is a padding */
		if (*pnt == 0x00)
		{
			pnt++;
			continue;
		}
		/* If the End-Of-Options has been reached */
		if (*pnt == 0xFF)
			break;
		/* If the current extension has the requested ID */
		if (*pnt == id)
		{
			result = pnt;
			break;
		}
		/* Go to next option */
		pnt += 2 + pnt[1];
	}
	return result;
}
/* EOF */
