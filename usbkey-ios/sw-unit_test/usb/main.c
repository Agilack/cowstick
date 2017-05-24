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
#include "usb.h"

static void clk_debug(void);

/**
 * @brief Entry point and main loop of the unit-test
 *
 */
int main(void)
{
	hw_init();

	/* DIR: Set PA15 as output */
	reg_wr(0x60000000 + 0x08, (1 << 15));
	/* Set LED "OFF" */
	reg_wr(0x60000000 + 0x18, (1 << 15));
	/* PINCFG: Configure PA15 (normal strength, no pull, no pmux) */
	reg8_wr(0x60000000 + 0x4F, 0x00);

	usb_init();

	usb_config();

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

/**
 * @brief USB peripheral interrupt handler
 *
 */
void USB_Handler(void)
{
	usb_irq();
	/* Toggle LED value */
	reg_wr(0x60000000 + 0x1C, (1 << 15));
}
/* EOF */
