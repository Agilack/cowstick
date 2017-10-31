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

typedef struct _network
{
	u8  *rx_buffer;
	int  rx_length;
	int  rx_state;
	u8  *tx_buffer;
	void (*tx_more)(struct _network *mod);
	/* Pointer to low-level driver */
	void *driver;
} network;

typedef struct __attribute__((packed))
{
	u8  dst[6];
	u8  src[6];
	u16 proto;
} eth_frame;

void net_init    (network *mod);
void net_periodic(network *mod);

#endif
