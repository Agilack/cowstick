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
static void wait(u32 cycles);
void mouse_enable(usb_module *mod);
void mouse_setup (usb_module *mod);
void mouse_xfer  (usb_module *mod, u8 ep);

usb_module mod;
usb_class  mouse_class;

int ready;
u8 ep1[64];

/**
 * @brief Entry point and main loop of the unit-test
 *
 */
int main(void)
{
	int step;
	int count;

	ready = 0;

	hw_init();

	usb_init();

	memset(&mouse_class, 0, sizeof(usb_class));
	mouse_class.enable = mouse_enable;
	mouse_class.setup  = mouse_setup;
	mouse_class.xfer   = mouse_xfer;

	memset(&mod, 0, sizeof(usb_module));
	mod.desc  =  mouse_desc_array;
	mod.class = &mouse_class;
	usb_config(&mod);

	step = 0;
	count = 0;
	while (1)
	{
		/* If HID not ready, nothing to do */
		if ( ! ready)
			continue;

		/* Wait a small amount of time */
		wait(0x40000);
		/* Set hid-busy flag */
		ready = 0;

		ep1[0] =    0; // Buttons
		/* Move right */
		if (step == 0)
		{
			ep1[1] = 0x2; // X
			ep1[2] = 0x0; // Y
		}
		/* Move down */
		else if (step == 1)
		{
			ep1[1] = 0x00; // X
			ep1[2] = 0x02; // Y
		}
		/* Move left */
		else if (step == 2)
		{
			ep1[1] = 0xfe; // X
			ep1[2] = 0x00; // Y
		}
		/* Move up */
		else if (step == 3)
		{
			ep1[1] = 0x00; // X
			ep1[2] = 0xfe; // Y
		}
		/* Do nothing (wait) */
		else if (step == 4)
		{
			ep1[1] = 0x00; // X
			ep1[2] = 0x00; // Y
		}
		ep1[3] =    0; // Wheel
		usb_transfer(&mod, 0x81, ep1, 4);

		count ++;
		if (count == 50)
		{
			count = 0;
			step ++;
		}
		if (step == 5)
			step = 0;
	}
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

void mouse_enable(usb_module *mod)
{
	/* Enable endpoint 1 for datas */
	usb_ep_enable(mod, 1, 0x40);

	ready = 1;
}

void mouse_setup(usb_module *mod)
{
	usb_transfer(mod, 0x80, 0, 0);
}

void mouse_xfer(usb_module *mod, u8 ep)
{
	ready = 1;
}

/**
 * @brief USB peripheral interrupt handler
 *
 */
void USB_Handler(void)
{
	usb_irq(&mod);
}

/**
 * @brief Wait a short amount of time (simple empty loop)
 *
 * @param cycles Number of "nop" loop before return
 */
static void wait(u32 cycles)
{
	while(cycles)
		cycles--;
}
/* EOF */
