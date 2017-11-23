/**
 * @file  net_dhcp.h
 * @brief Definitions and prototypes for DHCP network service
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
#ifndef NET_DHCP_H
#define NET_DHCP_H
#include "log.h"
#include "types.h"
#include "net_ipv4.h"

#ifdef DEBUG_DHCP
#define DHCP_PUTS(x) DBG_PUTS(x)
#else
#define DHCP_PUTS(x) {}
#endif

#define DHCP_DISCOVER 1
#define DHCP_OFFER    2

typedef struct __attribute__((packed))
{
	u8  op;
	u8  htype;
	u8  hlen;
	u8  hops;
	u32 xid;
	u16 secs;
	u16 flags;
	u32 ciaddr;
	u32 yiaddr;
        u32 siaddr;
        
        u32 giaddr;
        u8  chaddr[16];
        u8  sname [64];
        u8  file [128];
} dhcp_packet;

void dhcp_recv(network *netif, udp_packet *pkt, ip_dgram *ip);

#endif
/* EOF */
