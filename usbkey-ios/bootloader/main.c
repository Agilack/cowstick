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
#include "uart.h"
#include "usb.h"

void Jumper(u32 fct, u32 stack);
static void bootloader(void);

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

/**
 * @brief Main function when start in bootloader mode
 *
 */
static void bootloader(void)
{
	/* Initialize UART debug port */
	uart_init();
	/* Initialize USB stack */
	usb_init();

	led_status(0x00020006);

	/* Infinite loop, do nothing */
	while(1)
		;
}

/* EOF */
