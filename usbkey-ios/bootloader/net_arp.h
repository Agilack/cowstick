/**
 * @file  net_arp.h
 * @brief Definitions and prototypes for ARP network protocol
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
#ifndef NET_ARP_H
#define NET_ARP_H
#include "net.h"
#include "types.h"

#ifdef DEBUG_ARP
#define ARP_PUTS(x) DBG_PUTS(x)
#else
#define ARP_PUTS(x) {}
#endif

typedef struct __attribute__((packed))
{
	u16 type;
	u16 proto;
	u8  hlen;
	u8  llen;
	u16 op;
	u8  src_phy[6];
	u32 src_ip;
	u8  dst_phy[6];
	u32 dst_ip;
} arp_packet;

void arp_receive(network *mod, u8 *data, int length);

#endif
