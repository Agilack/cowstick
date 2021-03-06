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
#include "log.h"
#include "types.h"

#ifdef DEBUG_NET
#define NET_PUTS(c) DBG_PUTS(x)
#else
#define NET_PUTS(x) {}
#endif

#define IP_PROTO_ICMP 01
#define IP_PROTO_IGMP 02
#define IP_PROTO_TCP  06
#define IP_PROTO_UDP  17

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

/* -------------------------------------------------------------------------- */
/*                                    TCP                                     */
/* -------------------------------------------------------------------------- */

#define TCP_FIN 0x01
#define TCP_SYN 0x02
#define TCP_RST 0x04
#define TCP_PSH 0x08
#define TCP_ACK 0x10

typedef struct __attribute__((packed))
{
	u16 src_port;
	u16 dst_port;
	u32 seq;
	u32 ack;
	u8  offset;
	u8  flags;
	u16 win;
	u16 cksum;
	u16 urg;
} tcp_packet;

#define TCP_CONN_CLOSED      0
#define TCP_CONN_SYN         1
#define TCP_CONN_ESTABLISHED 2
#define TCP_CONN_CLOSE_WAIT  3
#define TCP_CONN_FIN_WAIT_1  4
#define TCP_CONN_CLOSING     5

typedef struct _tcp_conn
{
	u32 ip_remote;
	u16 port_local;
	u16 port_remote;
	u32 seq_local;
	u32 seq_remote;
	u8  state;
	tcp_packet *req;
	tcp_packet *rsp;
	struct _network  *netif;
	struct _tcp_service *service;
	int (*closed) (struct _tcp_conn *conn);
	int (*process)(struct _tcp_conn *conn, u8 *data, int len);
	int (*tx_more)(struct _tcp_conn *conn);
	void *priv;
} tcp_conn;

typedef struct _tcp_service
{
	u16   port;
	int (*accept) (tcp_conn *conn);
	int (*closed) (tcp_conn *conn);
	int (*process)(tcp_conn *conn, u8 *data, int len);
	void *priv;
} tcp_service;

void tcp4_close(tcp_conn *conn);
void tcp4_send (tcp_conn *conn, int len);
u8  *tcp4_tx_buffer(tcp_conn *conn);

/* -------------------------------------------------------------------------- */
/*                                    UDP                                     */
/* -------------------------------------------------------------------------- */

typedef struct __attribute__((packed))
{
	u16 src_port;
	u16 dst_port;
	u16 length;
	u16 cksum;
} udp_packet;

typedef struct _udp_conn
{
	u32 ip_remote;
	u16 port_local;
	u16 port_remote;
	udp_packet *rsp;
} udp_conn;

void udp4_send(network *mod, udp_conn *conn, int len);
u8  *udp4_tx_buffer(network *mod, udp_conn *conn);

#endif
