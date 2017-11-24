/**
 * @file  main.c
 * @brief Main function of the cowstick bootloader
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
#include "hardware.h"
#include "libc.h"
#include "net.h"
#include "net_ipv4.h"
#include "uart.h"
#include "usb.h"
#include "usb_ecm.h"
#include "usb_desc.h"

void Jumper(u32 fct, u32 stack);
static void bootloader(void);

usb_module usbmod;

static int echo_accept (tcp_conn *conn);
static int echo_closed (tcp_conn *conn);
static int echo_process(tcp_conn *conn, u8 *data, int len);

/**
 * @brief Entry point of the bootloader
 *
 */
int main(void)
{
	/* Initialize low-level hardware*/
	hw_init();

	/* If button is pressed at power on, start into Bootloader mode */
	if (button_status())
		bootloader();
	/* Else, start the main firmware */
	else
	{
		/* Get stack address from firmware vector 0 */
		u32 stack   = *(u32 *)0x00004000;
		/* Get firmware entry point from vector 1 */
		u32 handler = *(u32 *)0x00004004;

		/* In case of invalid stack address, start bootloader */
		if ((stack < 0x20000000) || (stack > 0x20008000))
			bootloader();

		led_status(0x00080010);

		/* Update Vector Table Offset Register */
		reg_wr(0xE000ED08, 0x00004000);

		/* Go ! Go ! Go ! */
		Jumper(handler, stack);
	}

	/* Never comes here */
	return(0);
}

/* Bootloader variables */
static u8 bl_net_rx_buffer[512];
static u8 bl_net_tx_buffer[512];

/**
 * @brief Main function when start in bootloader mode
 *
 */
static void bootloader(void)
{
	usb_class   ecm_class;
	network     net_cfg;
	tcp_conn    tcp_conns[2];
	tcp_service tcp_services;

	/* Initialize UART debug port */
	uart_init();
	/* Initialize USB stack */
	usb_init();

	uart_puts("--=={ Cowstick Bootloader }==--\r\n");

	/* DEBUG: Dummy service used to test TCP layer */
	tcp_services.port    = 1234;
	tcp_services.accept  = echo_accept;
	tcp_services.closed  = echo_closed;
	tcp_services.process = echo_process;
	tcp_services.priv    = 0;

	/* Init TCP connections */
	net_cfg.tcp.conns = &tcp_conns[0];
	net_cfg.tcp.conn_count = 2;
	/* Init TCP services */
	net_cfg.tcp.services = &tcp_services;
	net_cfg.tcp.service_count = 1;
	/* Initialize network interface */
	net_init(&net_cfg);
	/* Configure network interface : set RX/TX buffers */
	net_cfg.rx_buffer = bl_net_rx_buffer;
	net_cfg.rx_length = 0;
	net_cfg.rx_state  = 0;
	net_cfg.tx_buffer = bl_net_tx_buffer;
	net_cfg.tx_more   = 0;
	/* Save pointer to USB ECM driver */
	net_cfg.driver    = (void *)&usbmod;

	/* Configure USB device (and attach it) */
	memset(&usbmod, 0, sizeof(usb_module));
	usbmod.desc = (u8*)usb_ecm_desc;
	ecm_init(&usbmod, &ecm_class);
	ecm_class.priv = (void *)&net_cfg;
	usb_config(&usbmod);

	led_status(0x00020006);

	uart_puts("Bootloader: ready\r\n");

	/* Infinite loop for firmware events */
	while(1)
	{
		net_periodic(&net_cfg);
	}
}

/**
 * @brief USB peripheral interrupt handler
 *
 */
void USB_Handler(void)
{
	usb_irq(&usbmod);
}

static int echo_accept(tcp_conn *conn)
{
	(void)conn;

	uart_puts("Dummy service ECHO - Accept\r\n");

	return 0;
}

static int echo_closed(tcp_conn *conn)
{
	(void)conn;

	uart_puts("Dummy service ECHO - Connection closed\r\n");

	return 0;
}

static int echo_more(tcp_conn *conn)
{
	u8 *buffer = tcp4_tx_buffer(conn);

	conn->tx_more = 0;

	memcpy(buffer, "Echo:\0", 6);
	tcp4_send(conn, 6);

	return(1);
}

static int echo_process(tcp_conn *conn, u8 *data, int len)
{
	uart_puts("Dummy service ECHO - Recv\r\n");
	uart_dump(data, len);

	if (data[0] == ' ')
	{
		u8 *buffer = tcp4_tx_buffer(conn);

		conn->tx_more = echo_more;

		memcpy(buffer, "Multiline demo\r\n\0", 17);
		tcp4_send(conn, 17);
	}
	else if (data[0] == 'd')
	{
		uart_dump((u8 *)0x20007000, 0x1000);
	}
	else if (data[0] == 'q')
	{
		u8 *buffer = tcp4_tx_buffer(conn);

		memcpy(buffer, "Bye !\r\n\0", 8);
		tcp4_send(conn, 8);

		uart_puts("Echo quit\r\n");
		tcp4_close(conn);
	}
	else
	{
		u8 *buffer = tcp4_tx_buffer(conn);

		memcpy(buffer, "Echo> \0", 7);
		tcp4_send(conn, 7);
	}

	return 0;
}

/* EOF */
