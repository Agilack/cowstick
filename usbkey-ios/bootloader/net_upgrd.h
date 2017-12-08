/**
 * @file  net_upgrd.h
 * @brief Definitions and prototypes for Socket-Upgrade service
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
#ifndef NET_UPGRD_H
#define NET_UPGRD_H
#include "log.h"
#include "net.h"
#include "net_ipv4.h"

#ifdef DEBUG_UPGRD
#define UPGRD_PUTS(x) DBG_PUTS(x)
#else
#define UPGRD_PUTS(x) {}
#endif

typedef struct _upgrd
{
	int status;
	u32 offset;
	u8  cache_len;
	u8  cache[64];
} upgrd;

void upgrd_init(tcp_service *srv, upgrd *session);

int  upgrd_accept(tcp_conn *conn);
int  upgrd_closed(tcp_conn *conn);
int  upgrd_recv  (tcp_conn *conn, u8 *data, int len);
#endif
