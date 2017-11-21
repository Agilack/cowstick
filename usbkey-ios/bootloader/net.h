/**
 * @file  net.h
 * @brief Definitions and prototypes for network interface
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
#ifndef NET_H
#define NET_H
#include "types.h"

/* Set the local IP address (if not already defined) */
#ifndef CFG_IP_LOCAL
#define CFG_IP_LOCAL 0x0A0A0AFE
#endif
/* Set the remote IP address (if not already defined) */
#ifndef CFG_IP_REMOTE
#define CFG_IP_REMOTE 0x0A0A0A03
#endif

typedef struct _network
{
	u8  *rx_buffer;
	int  rx_length;
	int  rx_state;
	u8  *tx_buffer;
	void (*tx_more)(struct _network *mod);
	/* Pointer to low-level driver */
	void *driver;
	/* MAC address of the interface */
	u8    mac[6];
	/* Extension for TCP */
	struct
	{
		struct _tcp_conn *conns;
		int    conn_count;
	} tcp;
} network;

typedef struct __attribute__((packed))
{
	u8  dst[6];
	u8  src[6];
	u16 proto;
} eth_frame;

u32  htonl(u32 v);
u16  htons(u16 v);
void net_init    (network *mod);
void net_periodic(network *mod);
void net_send(network *mod, u32 size);
u8*  net_tx_buffer(network *mod, u16 proto);

#endif
