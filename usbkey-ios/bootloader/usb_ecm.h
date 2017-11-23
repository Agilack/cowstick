/**
 * @file  usb_ecm.h
 * @brief Definitions for USB Ethernet Control Model (ECM)
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
#ifndef USB_ECM_H
#define USB_ECM_H
#include "log.h"
#include "usb.h"

#ifdef DEBUG_USB
#define ECM_PUTS(x) DBG_PUTS(x)
#else
#define ECM_PUTS(x) {}
#endif

void ecm_init(usb_module *mod, usb_class *obj);
void ecm_rx_prepare(usb_module *mod);
void ecm_tx(usb_module *mod, u8 *buffer, u32 size);
#endif
