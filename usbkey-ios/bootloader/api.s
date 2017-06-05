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
	.long 0x12345678
	.long 0
	.long 0
	.long 0

