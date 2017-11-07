/**
 * @file  net_ipv4.h
 * @brief Definitions and prototypes for IPv4 network protocol
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
#ifndef NET_IPV4_H
#define NET_IPV4_H

#include "types.h"

typedef struct __attribute__((packed))
{
	u8  vihl;
	u8  tos;
	u16 length;
	u16 id;
	u16 offset;
	u8  ttl;
	u8  proto;
	u16 cksum;
	u32 src;
	u32 dst;
	u8  data;
} ip_dgram;

struct _network;

u16  ip_cksum(u32 sum, const u8 *data, u16 len);
void ipv4_init(struct _network *mod);
void ipv4_receive(struct _network *mod, u8 *buffer, int length);
void ipv4_send(network *mod, int len);
u8  *ipv4_tx_buffer(network *mod, u32 dest, u8 proto);

#endif
