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
#include "uart.h"

/**
 * @brief Entry point and main loop of the unit-test
 *
 */
int main(void)
{
	hw_init();

	uart_init();
	
	uart_puts("--=={ Cowstick UART unit-test }==--\r\n");

	while (1) 
	{
		int i;
		/* Send a byte to UART */
		uart_putc('*');
		/* Short wait */
		for (i = 0; i < 0x20000; i++)
			;
	}
}
/* EOF */
