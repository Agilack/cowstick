/**
 * @file  net_upgrd.c
 * @brief Implement Socket-Upgrade service
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
#include "flash.h"
#include "hardware.h"
#include "libc.h"
#include "net_upgrd.h"
#include "uart.h"

static void upgrd_write(u32 addr, u8 *data);

/**
 * @brief Initialize the Socket-Upgrade service
 *
 * @param srv     Pointer to th associated TCP service structure
 * @param session Pointer to an upgrd structure used for private datas
 */
void upgrd_init(tcp_service *srv, upgrd *session)
{
	/* Configure the service */
	if (srv != 0)
	{
		srv->port    = 1234;
		srv->accept  = upgrd_accept;
		srv->closed  = upgrd_closed;
		srv->process = upgrd_recv;
		srv->priv    = session;
	}
	/* Initialize the default session */
	if (session)
	{
		session->status = 0;
		session->offset = 0;
		session->cache_len = 0;
	}
}

/**
 * @brief Called by TCP/IP module to handle an incoming connection
 *
 * @param conn Pointer to the associated TCP connection
 * @return Return 0 to accept connection, any other value to reject
 */
int upgrd_accept(tcp_conn *conn)
{
	tcp_service *srv;
	upgrd *session;

	uart_puts(" * Upgrade: start\r\n");

	srv = conn->service;
	if ( (srv == 0) || (srv->priv == 0) )
		return(1);

	session = (upgrd *)srv->priv;

	/* If another session is already connected ... */
	if (session->status != 0)
		/* then reject request : only one socket at a time */
		return(1);

	/* Configure session */
	session->status = 1; /* Set session to "connected" */ 
	session->offset = 0;
	session->cache_len = 0;

	/* Save session into connection descriptor */
	conn->priv = (void *)session;

	return(0);
}

/**
 * @brief Called by TCP/IP module when a connection is closed
 *
 * @param conn Pointer to the associated TCP connection
 * @return Return value not used (reserved for future use)
 */
int upgrd_closed(tcp_conn *conn)
{
	upgrd *session;

	uart_puts(" * Upgrade: finished\r\n");

	session = (upgrd *)conn->priv;
	if (session->cache_len > 0)
	{
		u32 dst;
		int i;
		/* Clear end of the cache */
		for (i = session->cache_len; i < 64; i++)
			session->cache[i] = 0xFF;
		/* Set destination address */
		dst  = 0x00004000;
		dst += session->offset;
		/* Write last page */
		upgrd_write(dst, session->cache);
	}
	/* Update status : Disconnected */
	session->status = 0;

#ifdef DEBUG_UPGRD
	uart_dump((u8 *)0x00004000, 1524);
#endif

	led_status(0x00010002);

	return(0);
}

/**
 * @brief Called by TCP/IP moduel when datas are received on connection
 *
 * @param conn Pointer to the associated TCP connection
 * @param data Pointer to the received data buffer
 * @param len  Length (in bytes) of the received packet
 * @return Return value not used (reserved for future use)
 */
int upgrd_recv(tcp_conn *conn, u8 *data, int len)
{
	upgrd *session;
	u8 *psrc;
	int plen;
	u32 dst;

	session = (upgrd *)conn->priv;
	psrc = data;
	plen = len;
	/* Set destination address */
	dst  = 0x00004000;
	dst += session->offset;

	/* First, process datas from cache */
	if (session->cache_len > 0)
	{
		int clen;
		u8 *pdst = session->cache;
		pdst += session->cache_len;

		/* Compute length to fill cache */
		clen = (64 - session->cache_len);
		if (clen > plen)
			clen = plen;

		/* Copy data to complete cache */
		memcpy(pdst, psrc, clen);
		/* Update cache length */
		session->cache_len += clen;

		if (session->cache_len == 64)
		{
			upgrd_write(dst, session->cache);
			session->offset += 64;
			dst += 64;
			session->cache_len = 0;
		}

		/* Update length and pointers */
		psrc += clen;
		plen -= clen;
	}

	/* Second, process full pages */
	while(plen >= 64)
	{
		/* Write a full page */
		upgrd_write(dst, psrc);
		/* Update destination address */
		session->offset += 64;
		dst += 64;
		/* Update length */
		psrc += 64;
		plen -= 64;
	}

	if (plen > 0)
	{
		memcpy(session->cache, psrc, plen);
		session->cache_len = plen;
	}

	return(0);
}

/**
 * @brief Write one page, erase before if necessary
 *
 * @param addr Start address of the page to write
 * @param data Pointer to the data to write (source)
 */
static void upgrd_write(u32 addr, u8 *data)
{
	/* If the target address is at the begining of a row : Erase it ! */
	if ((addr & 0xFF) == 0)
	{
#ifdef DEBUG_UPGRD
		uart_puts("Flash: erase row  ");
		uart_puthex(addr);
		uart_crlf();
#endif
		flash_erase(addr);
	}

	/* Write one page to flash */
#ifdef DEBUG_UPGRD
	uart_puts("NVM: write page ");
	uart_puthex(addr);
	uart_crlf();
#endif
	flash_write(addr, data);
}
/* EOF */
