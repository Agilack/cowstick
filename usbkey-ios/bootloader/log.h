/**
 * @file  usb.h
 * @brief Global definitions used for log and debug
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
#ifndef LOG_H
#define LOG_H

#include "uart.h"

#define DBG_PUTC(x)     uart_putc(x)
#define DBG_PUTS(x)     uart_puts(x)
#define DBG_PUTHEX8(x)  uart_puthex8(x)
#define DBG_PUTHEX16(x) uart_puthex16(x)

#endif
