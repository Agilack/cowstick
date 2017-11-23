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
static void tcp4_accept (network *netif, tcp_packet *req);
static tcp_packet *tcp4_prepare(tcp_conn *conn);
static void tcp4_receive(network *netif, tcp_packet *pkt, int len);
static void tcp4_tx_wait(tcp_conn *conn);
/* UDP functions */
static void udp4_receive(network *mod, udp_packet *pkt, ip_dgram *ip);

/**
 * @brief Initialize the IPv4 protocol module
 *
 * @param mod Pointer to the network interface that will use IPv4
 */
void ipv4_init(network *mod)
{
	int i;

	for (i = 0; i < mod->tcp.conn_count; i++)
	{
		mod->tcp.conns[i].ip_remote = 0;
		mod->tcp.conns[i].state     = TCP_CONN_CLOSED;
		mod->tcp.conns[i].process   = 0;
	}
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

/**
 * @brief Establish a new connection for a received request (accept)
 *
 * @param netif Pointer to the network interface structure
 * @param req   Pointer to the received TCP packet
 */
static void tcp4_accept(network *netif, tcp_packet *req)
{
	tcp_packet *rsp;
	tcp_conn   *newconn = 0;
	tcp_conn   tmpconn;
	ip_dgram *ip;
	u8  *buffer;
	int i;

	ip = (ip_dgram *)(((u8*)req) - 20);

	/* Create a new TCP connection for this network interface */
	for (i = 0; i < netif->tcp.conn_count; i++)
	{
		/* If the current slot is already configured, continue search */
		if (netif->tcp.conns[i].ip_remote != 0x00000000)
			continue;
		/* Select current array item for new connection */
		newconn = &netif->tcp.conns[i];

		/* Configure new connection */
		newconn->ip_remote  = htonl(ip->src);
		newconn->port_local = htons(req->dst_port);
		newconn->port_remote= htons(req->src_port);
		newconn->seq_local  = 0x12345678;
		newconn->seq_remote = htonl(req->seq) + 1;
		newconn->state      = TCP_CONN_SYN;
		newconn->netif      = netif;
		newconn->process    = 0;

		break;
	}

	/* Get the buffer of TX datagram from IPv4 underlayer */
	buffer = (u8 *)ipv4_tx_buffer(netif, htonl(ip->src), 0x06);

	rsp = (tcp_packet *)buffer;
	rsp->src_port = req->dst_port;
	rsp->dst_port = req->src_port;
	rsp->ack = htonl( htonl(req->seq) + 1);
	rsp->offset = 0x50;
	rsp->flags  = TCP_ACK;
	rsp->win    = htons(450);
	rsp->cksum  = 0x0000;
	rsp->urg    = 0x0000;

	if (newconn == 0)
		goto reject;

	/* Search a service for the requested target port */
	for (i = 0; i < netif->tcp.service_count; i++)
	{
		if (htons(req->dst_port) != netif->tcp.services[i].port)
			continue;
		newconn->process =  netif->tcp.services[i].process;
		newconn->service = &netif->tcp.services[i];
		break;
	}
	/* If no service found ... reject the request */
	if (newconn->process == 0)
		goto reject;

	if (newconn->service->accept != 0)
		newconn->service->accept(newconn);
	else
	{
		uart_puts(" * ACCEPT from=");
		uart_puthex(newconn->ip_remote);
		uart_puts(" local port=");  uart_puthex16(newconn->port_local);
		uart_puts(" remote port="); uart_puthex16(newconn->port_remote);
		uart_crlf();
	}

	rsp->flags |= TCP_SYN;
	rsp->seq    = htonl(newconn->seq_local);
	goto send;

reject:
	memset(&tmpconn, 0, sizeof(tcp_conn));
	newconn = &tmpconn;
	newconn->netif = netif;
	rsp->flags |= TCP_RST;
	rsp->seq    = 0x00000000;

send:
	newconn->rsp = rsp;
	tcp4_send(newconn, 0);
}

/**
 * @brief Start a close sequence, initiated by local side of connection
 *
 * @param conn  Pointer to the TCP connection to close
 */
void tcp4_close(tcp_conn *conn)
{
	network    *netif = 0;
	tcp_packet *rsp   = 0;

	/* Sanity check */
	if (conn == 0)
		return;

	/* Get associated network interface */
	netif = conn->netif;
	if (netif == 0)
		return;

	tcp4_tx_wait(conn);

	rsp = tcp4_prepare(conn);
	rsp->flags |= TCP_ACK | TCP_FIN;
	rsp->seq    = htonl(conn->seq_local);
	rsp->ack    = htonl(conn->seq_remote);
	/* Set the connection into CLOSE_WAIT state */
	conn->state = TCP_CONN_FIN_WAIT_1;
	/* Send response */
	tcp4_send(conn, 0);
}

/**
 * @brief Search if a packet is for an existing connection (from cache)
 *
 * @param netif Pointer to the network interface structure
 * @param pkt   Pointer to the received packet
 * @return Pointer to the TCP connection structure, or NULL if not found
 */
static tcp_conn *tcp4_find(network *netif, tcp_packet *pkt)
{
	tcp_conn *result = 0;
	u16 req_rem_port;
	u16 req_loc_port;
	int i;

	req_rem_port = htons(pkt->src_port);
	req_loc_port = htons(pkt->dst_port);

	for (i = 0; i < netif->tcp.conn_count; i++)
	{
		/* Use IP field to test if an entry is filled or empty */
		if (netif->tcp.conns[i].ip_remote == 0x00000000)
			continue;
		/* Test local port number field */
		if (netif->tcp.conns[i].port_local != req_loc_port)
			continue;
		/* Test remote port number field */
		if (netif->tcp.conns[i].port_remote != req_rem_port)
			continue;

		// ToDo : check IP

		result = &netif->tcp.conns[i];
		break;
	}
	return result;
}

/**
 * @brief Prepare a buffer for a TX packet
 *
 * @param netif Pointer to the network interface structure
 * @param conn  Pointer to the associated TCP connection
 * @return Pointer to the prepared TCP packet (or NULL in case of error)
 */
static tcp_packet *tcp4_prepare(tcp_conn *conn)
{
	network    *netif;
	tcp_packet *rsp;

	/* Sanity check */
	if (conn == 0)
		return(0);

	netif = conn->netif;

	tcp4_tx_wait(conn);

	rsp = (tcp_packet *)ipv4_tx_buffer(netif, conn->ip_remote, 0x06);
	rsp->src_port = 0;
	rsp->dst_port = 0;
	rsp->ack      = 0;
	rsp->offset   = 0x50;
	rsp->flags    = 0;
	rsp->win      = htons(256);
	rsp->cksum    = 0x0000;
	rsp->urg      = 0x0000;

	rsp->flags |= TCP_ACK;
	rsp->src_port = htons(conn->port_local);
	rsp->dst_port = htons(conn->port_remote);
	rsp->ack      = htonl(conn->seq_remote);
	rsp->seq      = htonl(conn->seq_local);

	conn->rsp = rsp;

	return rsp;
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
	tcp_packet *rsp = 0;
	tcp_conn   *conn;

	/* Search if the received packet refers to a known socket */
	conn = tcp4_find(netif, req);

	if ((conn != 0) && ( (conn->state == TCP_CONN_CLOSE_WAIT) ||
	                     (conn->state == TCP_CONN_CLOSING)))
	{
		if (req->flags & TCP_ACK)
		{
			uart_puts("TCP4: Connection closed\r\n");
			conn->ip_remote = 0;
			conn->state = TCP_CONN_CLOSED;
		}
	}
	else if ((conn != 0) && (conn->state == TCP_CONN_SYN))
	{
		if (req->flags & TCP_ACK)
		{
			uart_puts("TCP4: Connection established\r\n");
			conn->seq_local = htonl(req->ack);
			conn->state = TCP_CONN_ESTABLISHED;
		}
	}
	else if ((conn != 0) && (conn->state == TCP_CONN_FIN_WAIT_1))
	{
		/* If the received packet contains a ACK value */
		if (req->flags & TCP_ACK)
			/* Save it ! Note : Big security issue, but we assume to trust this link */
			conn->seq_local = htonl(req->ack);

		/* Update the (remote) sequence number */
		conn->seq_remote = htonl(req->seq);
		conn->seq_remote += 1;

		if (req->flags & TCP_FIN)
		{
			rsp = tcp4_prepare(conn);
			rsp->flags |= TCP_ACK;
			rsp->seq    = htonl(conn->seq_local);
			rsp->ack    = htonl(conn->seq_remote);
			/* Set the connection into CLOSE_WAIT state */
			conn->state = TCP_CONN_CLOSING;
			/* Send response */
			tcp4_send(conn, 0);

			tcp4_tx_wait(conn);

			uart_puts("TCP4: Connection closed\r\n");
			conn->ip_remote = 0;
			conn->state = TCP_CONN_CLOSED;
		}
	}
	/* Data packet received for a known connection */
	else if (conn != 0)
	{
		int dlen, hlen;

		/* Compute TCP header length */
		hlen = ((req->offset >> 2) & 0x3C);
		/* Compute data length */
		dlen = len - hlen;

		/* If the received packet contains a ACK value */
		if (req->flags & TCP_ACK)
		{
			/* Save it ! Note : Big security issue, but we assume to trust this link */
			conn->seq_local = htonl(req->ack);
		}

		if  ( (dlen > 0) || (req->flags & TCP_FIN) )
		{
			/* Update the (remote) sequence number */
			conn->seq_remote = htonl(req->seq);
			/* Update sequence number (remote) */
			if (dlen == 0)
				conn->seq_remote += 1;
			else
				conn->seq_remote += dlen;

			rsp = tcp4_prepare(conn);

			if (req->flags & 0x01)
			{
				rsp->flags |= TCP_ACK | TCP_FIN;
				rsp->seq    = htonl(conn->seq_local);
				rsp->ack    = htonl(conn->seq_remote);
				/* Set the connection into CLOSE_WAIT state */
				conn->state = TCP_CONN_CLOSE_WAIT;
			}

			/* Send response */
			tcp4_send(conn, 0);

			/* Wait end of transmit */
			tcp4_tx_wait(conn);
		}
		/* If the packet contains data, call application callback */
		if ( dlen > 0)
		{
			u8 *buffer = (u8 *)req + hlen;
			conn->req = req;
			conn->rsp = 0;
			conn->process(conn, buffer, dlen);
			conn->req = 0;
		}
	}
	else if (req->flags & TCP_SYN)
	{
		tcp4_accept(netif, req);
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
 * @param conn  Pointer to the TCP connection
 * @param len   Length of the datas into the packet
 */
void tcp4_send(tcp_conn *conn, int len)
{
	network *netif;
	u8 src_addr[4]  = {0x0A,0x0A,0x0A,0xFE};
	u8 dest_addr[4] = {0x0A,0x0a,0x0a,0x03};
	u16 *ip_src=(void *)&src_addr;
	u16 *ip_dst=(void *)&dest_addr;
	u32 sum = 0;
	int hlen, tmp_len;
	tcp_packet *pkt = conn->rsp;
	u16 *p = (u16 *)pkt;

	if (conn == 0)
		return;

	netif = conn->netif;

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

	/* Reset rsp pointer after sending packet */
	conn->rsp = 0;
	/* Update sequence number */
	conn->seq_local += len;
}

/**
 * @brief Get a buffer for datas to be sent
 *
 * @param conn Pointer to a TCP connection (optional)
 * @return Pointer to the allocated buffer
 */
u8 *tcp4_tx_buffer(tcp_conn *conn)
{
	tcp_packet *rsp;
	int  hlen;
	u8  *data;

	/* Sanity check */
	if (conn == 0)
		return (0);

	if (conn->rsp != 0)
		rsp = conn->rsp;
	else
	{
		rsp = tcp4_prepare(conn);
		// Save it into connection structure
		conn->rsp = rsp;
	}

	hlen = ( (rsp->offset >> 2) & 0x3C );
	data = ((u8*)rsp) + hlen;

	return data;
}

/**
 * @brief Test if TX buffer is ready for new packet, and wait if not empty
 *
 * @param conn Pointer to the TCP connection
 */
static void tcp4_tx_wait(tcp_conn *conn)
{
	network  *netif = conn->netif;
	volatile eth_frame *eth;
	u8 *dgram;

	/* Get pointer to current TX datagram */
	dgram = (u8 *)net_tx_buffer(netif, 0);
	/* Get the associated ethernet frame */
	eth   = (eth_frame *)(dgram - 14);

	/* Field 'proto' is cleared by USB ECM when frame sent */
	while (eth->proto != 0x0000)
		;
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
