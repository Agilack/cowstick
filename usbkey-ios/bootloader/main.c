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
usb_class  ecm_class;
u8 ep1[64];

static int echo_accept(tcp_conn *conn);
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
network  bl_net_cfg;
u8       bl_net_rx_buffer[512];
u8       bl_net_tx_buffer[512];

/**
 * @brief Main function when start in bootloader mode
 *
 */
static void bootloader(void)
{
	tcp_conn    bl_tcp_conns[2];
	tcp_service bl_tcp_services;

	/* Initialize UART debug port */
	uart_init();
	/* Initialize USB stack */
	usb_init();

	uart_puts("--=={ Cowstick Bootloader }==--\r\n");

	/* DEBUG: Dummy service used to test TCP layer */
	bl_tcp_services.port    = 1234;
	bl_tcp_services.accept  = echo_accept;
	bl_tcp_services.process = echo_process;
	bl_tcp_services.priv    = 0;

	/* Init TCP connections */
	bl_net_cfg.tcp.conns = &bl_tcp_conns[0];
	bl_net_cfg.tcp.conn_count = 2;
	/* Init TCP services */
	bl_net_cfg.tcp.services = &bl_tcp_services;
	bl_net_cfg.tcp.service_count = 1;
	/* Initialize network interface */
	net_init(&bl_net_cfg);
	/* Configure network interface : set RX/TX buffers */
	bl_net_cfg.rx_buffer = bl_net_rx_buffer;
	bl_net_cfg.rx_length = 0;
	bl_net_cfg.rx_state  = 0;
	bl_net_cfg.tx_buffer = bl_net_tx_buffer;
	bl_net_cfg.tx_more   = 0;
	/* Save pointer to USB ECM driver */
	bl_net_cfg.driver    = (void *)&usbmod;

	/* Configure USB device (and attach it) */
	memset(&usbmod, 0, sizeof(usb_module));
	usbmod.desc = (u8*)usb_ecm_desc;
	ecm_init(&usbmod, &ecm_class);
	ecm_class.priv = (void *)&bl_net_cfg;
	usb_config(&usbmod);

	led_status(0x00020006);

	uart_puts("Bootloader: ready\r\n");

	/* Infinite loop for firmware events */
	while(1)
	{
		net_periodic(&bl_net_cfg);
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
static int echo_process(tcp_conn *conn, u8 *data, int len)
{
	(void)conn;
	(void) data;
	(void)len;

	uart_puts("Dummy service ECHO - Recv\r\n");
	uart_dump(data, len);

	return 0;
}

/* EOF */
