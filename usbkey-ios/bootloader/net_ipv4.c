/**
 * @file  net_ipv4.c
 * @brief Implement IPv4 network protocol
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
#include "types.h"
#include "uart.h"

/* TCP functions */
static void tcp4_accept (network *netif, tcp_packet *req, int len);
static void tcp4_receive(network *netif, tcp_packet *pkt, int len);
/* UDP functions */
static void udp4_receive(network *mod, udp_packet *pkt, ip_dgram *ip);

/**
 * @brief Initialize the IPv4 protocol module
 *
 * @param mod Pointer to the network interface that will use IPv4
 */
void ipv4_init(network *mod)
{
	(void)mod;
}

/**
 * @brief Called by network layer when an IPv4 datagram is received
 *
 * @param mod    Pointer to the network interface structure
 * @param buffer Pointer to the data buffer where received datagram is stored
 * @param length Size of the received datagram (in bytes)
 */
void ipv4_receive(network *mod, u8 *buffer, int length)
{
	(void)length;
	ip_dgram *req = (ip_dgram *)buffer;

	/* Sanity check */
	if ((mod == 0) || (buffer == 0))
	{
		uart_puts("IPv4: Missing parameter for receive function\r\n");
		return;
	}

	/* Process datagram according to the IP protocol used */
	switch (req->proto)
	{
		case IP_PROTO_ICMP:
			uart_puts("IPv4: receive an ICMP packet\r\n");
			break;
		case IP_PROTO_IGMP:
			/* Not used yet */
			break;
		case IP_PROTO_UDP:
		{
			udp_packet *pkt = (udp_packet *)(buffer + 20);
			udp4_receive(mod, pkt, req);
			break;
		}
		case IP_PROTO_TCP:
		{
			tcp_packet *pkt = (tcp_packet *)(buffer + 20);
			tcp4_receive(mod, pkt, length - 20);
			break;
		}
		default:
			uart_puts("IPv4:");
			uart_puts(" src="); uart_puthex( htonl(req->src) );
			uart_puts(" dst="); uart_puthex( htonl(req->dst) );
			uart_puts(" proto="); uart_puthex8( req->proto );
			uart_crlf();
	}

}

/**
 * @brief Transmit an IPv4 datagram
 *
 * @param mod Pointer to the network interface structure
 * @param len Size of the datagram to send (in bytes)
 */
void ipv4_send(network *mod, int len)
{
	ip_dgram *rsp;
	u16 cksum;

	/* Get the buffer of TX frame from network layer */
	rsp = (ip_dgram *)net_tx_buffer(mod, 0);

	/* Update datagram length */
	rsp->length = htons(20 + len);
	/* Update the IP datagram checksum */
	cksum = ip_cksum(0, (u8 *)rsp, 20);
	rsp->cksum = htons(~cksum);

	/* Call underlying net layer to send datagram */
	net_send(mod, len + 20);
}

/**
 * @brief Get a pointer on a buffer that can be used to transmit a datagram
 *
 * @param mod   Pointer to the network interface structure
 * @param dest  IP address of the remote peer
 * @param proto ID of the protocol used inside ehe datagram
 */
u8 *ipv4_tx_buffer(network *mod, u32 dest, u8 proto)
{
	ip_dgram  *rsp;

	/* Get the buffer of TX frame from network layer */
	rsp = (ip_dgram *)net_tx_buffer(mod, 0x800);
	if (rsp == 0)
		return 0;
	
	rsp->vihl   = 0x45;
	rsp->tos    = 0x00;
	rsp->length = 0x0000;
	rsp->id     = 0x0000;
	rsp->offset = 0;
	rsp->ttl    = 0x40;
	rsp->proto  = proto;
	rsp->cksum  = 0x0000;
	rsp->src    = htonl(CFG_IP_LOCAL);
	rsp->dst    = htonl(dest);
	
	return (((u8 *)rsp) + 20);
}

/**
 * @brief Compute a 16bits checksum (mainly for IPv4 header)
 *
 * @param sum  Initial value for computation
 * @param data Pointer to the data buffer
 * @param len  Length of the data buffer
 */
u16 ip_cksum(u32 sum, const u8 *data, u16 len)
{
	u16 t;
	const u8 *dataptr;
	const u8 *last_byte;

	dataptr = data;
	last_byte = data + len - 1;

	/* At least two more bytes */
	while(dataptr < last_byte)
	{	
		t = (dataptr[0] << 8) + dataptr[1];
		sum += t;
//		if(sum < t)
//			sum++; /* carry */
		dataptr += 2;
	}
	
	if (len & 1)
	{
		t = (dataptr[0] << 8) + 0;
		sum += t;
//		if(sum < t)
//			sum++; /* carry */
      }

	while (sum & 0xffff0000)
	{
		u16 plop;
		plop = (sum >> 16);
		sum = (sum & 0xFFFF) + plop;
	}
	
	/* Return sum in host byte order. */
	return (u16)sum;
}

/* ------------------------------------------------------------------------- */
/* --                                 TCP                                 -- */
/* ------------------------------------------------------------------------- */

static void tcp4_accept(network *netif, tcp_packet *req, int len)
{
	tcp_packet *rsp;
	tcp_conn   newconn;
	ip_dgram *ip;
	u8  *buffer;

	uart_puts("TCP: ACCEPT\r\n");
	(void)len;

	ip = (ip_dgram *)(((u8*)req) - 20);

	newconn.ip_remote  = htonl(ip->src);
	newconn.port_local = htons(req->dst_port);
	newconn.port_remote= htons(req->src_port);
	newconn.seq_local  = 0x12345678;
	newconn.seq_remote = htonl(req->seq) + 1;
	newconn.state      = TCP_CONN_SYN;

	/* Get the buffer of TX datagram from IPv4 underlayer */
	buffer = (u8 *)ipv4_tx_buffer(netif, newconn.ip_remote, 0x06);

	rsp = (tcp_packet *)buffer;
	rsp->src_port = req->dst_port;
	rsp->dst_port = req->src_port;
	rsp->ack = htonl( htonl(req->seq) + 1);
	rsp->offset = 0x50;
	rsp->flags  = TCP_ACK;
	rsp->win    = htons(450);
	rsp->cksum  = 0x0000;
	rsp->urg    = 0x0000;

	rsp->flags |= TCP_SYN;
	rsp->seq    = htonl(1);

	newconn.rsp = rsp;
	tcp4_send(netif, &newconn, 0);
}

/**
 * @brief Process incoming TCP packet
 *
 * @param netif Pointer to the network interface structure
 * @param req   Pointer to the received TCP packet
 * @param len   Length of the received datas
 */
static void tcp4_receive(network *netif, tcp_packet *req, int len)
{
	uart_puts("TCP: receive\r\n");
	if (req->flags & TCP_SYN)
	{
		tcp4_accept(netif, req, len);
	}
	else
	{
		int size = len - sizeof(tcp_packet);
		if (size > 0)
		{
			u8 *buffer = (u8 *)req;
			buffer += sizeof(tcp_packet);
			uart_dump(buffer, size);
		}
	}
}

/**
 * @brief Send a TCP packet to a remote host
 *
 * @param netif Pointer to the network interface structure
 * @param conn  Pointer to the TCP connection
 * @param len   Length of the datas into the packet
 */
void tcp4_send(network *netif, tcp_conn *conn, int len)
{
	u8 src_addr[4]  = {0x0A,0x0A,0x0A,0xFE};
	u8 dest_addr[4] = {0x0A,0x0a,0x0a,0x03};
	u16 *ip_src=(void *)&src_addr;
	u16 *ip_dst=(void *)&dest_addr;
	u32 sum = 0;
	int hlen, tmp_len;
	tcp_packet *pkt = conn->rsp;
	u16 *p = (u16 *)pkt;

	uart_puts("TCP: SEND");

	/* Compute the TCP header length */
	hlen = (pkt->offset >> 2) & 0x3C;

	/* If packet contains datas, include the PUSH flag */
	if (len > 0)
		pkt->flags |= TCP_PSH;

	/* Reset checksum value */
	pkt->cksum = 0x0000;

	/* Sum all values of TCP header and datas */
	tmp_len = hlen + len;
	while (tmp_len > 1)
	{
		sum += *p++;
		if (sum & 0x80000000)
			sum = (sum & 0xFFFF) + (sum >> 16);
		tmp_len -= 2;
	}
	if (tmp_len & 1)
		sum += *((u8 *)p);
	/* Add the pseudo-header sum */
	sum += *(ip_src++);
	sum += *ip_src;
	sum += *(ip_dst++);
	sum += *ip_dst;
	sum += htons(0x06);
	sum += htons(hlen + len);
	/* Add the carries */
	while (sum >> 16)
		sum = (sum & 0xFFFF) + (sum >> 16);
	/* Set the computed checksum into TCP header */
	pkt->cksum = (u16)(~sum);

	/* Call underlying IP layer to send the packet */
	ipv4_send(netif, len + sizeof(tcp_packet));
}

/* ------------------------------------------------------------------------- */
/* --                                 UDP                                 -- */
/* ------------------------------------------------------------------------- */

/**
 * @brief Called by IPv4 layer when an UDP packet is received
 *
 * @param pkt Pointer to the received UDP packet
 *
 */
static void udp4_receive(network *mod, udp_packet *pkt, ip_dgram *ip)
{
	if (htons(pkt->dst_port) == 0x43)
		dhcp_recv(mod, pkt, ip);
#ifdef NET_UDP_DEBUG
	else
	{
		int i;

		uart_puts("UDP");
		uart_puts(" src_port="); uart_puthex16( htons(pkt->src_port) );
		uart_puts(" dst_port="); uart_puthex16( htons(pkt->dst_port) );
		uart_crlf();
		i = ip->length;
		if (i > 32)
			i = 32;
		uart_dump((u8 *)pkt, i);
	}
#endif
}

/**
 * @brief Transmit an UDP packet
 *
 * @param conn Pointer to the UDP connection structure
 * @param len  Size of the packet to send (in bytes)
 */
void udp4_send(network *mod, udp_conn *conn, int len)
{
	/* Update UDP header with packet length */
	udp_packet *pkt = (udp_packet *)conn->rsp;
	pkt->length = htons(8 + len);
	pkt->cksum  = 0;

	/* Call underlying IP layer to send the packet */
	ipv4_send(mod, len + sizeof(udp_packet));
}

/**
 * @brief Get a pointer on a buffer that can be used to transmit an UDP packet
 *
 * @param mod  Pointer to the network interface structure
 * @param conn Pointer to the UDP connection structure
 */
u8 *udp4_tx_buffer(network *mod, udp_conn *conn)
{
	udp_packet *rsp = 0;
	u8 *data;

	if (conn->rsp)
		rsp = conn->rsp;
	else
	{
		rsp = (udp_packet *)ipv4_tx_buffer(mod, conn->ip_remote, 0x11);
		rsp->src_port = conn->port_local;
		rsp->dst_port = conn->port_remote;
		conn->rsp = rsp;
	}

	data = ((u8*)rsp) + 8;

	return data;
}
/* EOF */
