/**
 * @file  main.c
 * @brief Main function of the unit-test
 *
 * @author Saint-Genest Gwenael <gwen@cowlab.fr>
 * @copyright Cowlab (c) 2017
 *
 * @page License
 * This unit-test is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation. You should have received a
 * copy of the GNU General Public License along with this program, see
 * LICENSE.md file for more details.
 * This program is distributed WITHOUT ANY WARRANTY.
 */
#include "hardware.h"
#include "libc.h"
#include "usb.h"
#include "usb_desc.h"

static void clk_debug(void);
void mouse_setup(usb_module *mod);

usb_module mod;
usb_class  mouse_class;

/**
 * @brief Entry point and main loop of the unit-test
 *
 */
int main(void)
{
	hw_init();

	usb_init();

	memset(&mouse_class, 0, sizeof(usb_class));
	mouse_class.setup = mouse_setup;

	memset(&mod, 0, sizeof(usb_module));
	mod.desc  =  mouse_desc_array;
	mod.class = &mouse_class;
	usb_config(&mod);

	while (1)
		;
}

/**
 * @brief Connect 48MHz DFLL to test-pin
 *
 */
static void clk_debug(void)
{
	u32 v;

	/* GCLK7 bit 19 must be set ! */

	/* Set PA23 as output */
	reg_wr(0x60000000 + 0x08, (1 << 23));
	/* PINCFG: enable PMUX to connect pin to GCLK */
	reg8_wr(0x60000000 + 0x57, 0x41);
	/* PINMUX : use Alternate Function H (GCLK) */
	v  = reg8_rd(0x60000000 + 0x3B);
	v &= 0x0F;
	v |= (0x07 << 4);
	reg8_wr(0x60000000 + 0x3B, v);
}

void mouse_setup(usb_module *mod)
{
	/* Nothing to do :) */
}

/**
 * @brief USB peripheral interrupt handler
 *
 */
void USB_Handler(void)
{
	usb_irq(&mod);
}
/* EOF */
