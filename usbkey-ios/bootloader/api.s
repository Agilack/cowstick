/**
 * @file  api.s
 * @brief Table of pointers for API entries
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

.syntax unified
.code 16

.section .api
.align 4

api_global: /* Offset 0xC0 */
	.long 0xDEADBEEF
	.long 0
	.long 0
	.long led_status

api_libc: /* Offset 0xD0 */
	.long memset
	.long memcpy
	.long 0
	.long 0

api_usb: /* Offset 0xE0 */
	.long usb_config
	.long usb_irq
	.long usb_transfer
	.long usb_ep_enable

api_uart: /* Offset 0xF0 */
	.long uart_init
	.long uart_puts
	.long uart_puthex
	.long 0

api_net: /* Offset 0x100 */
	.long net_init
