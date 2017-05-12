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

/**
 * @brief Entry point and main loop of the unit-test
 *
 */
int main(void)
{
	hw_init();

	/* Loop to blink LED (approx 10Hz) */
	while (1) 
	{
		int i;

#ifdef XPLAINED
		/* Toggle LED */
		reg_wr(0x60000080 + 0x1C, (1 << 30));
#else
		/* Toggle LED */
		reg_wr(0x60000000 + 0x1C, (1 << 15));
#endif
		/* Short wait */
		for (i = 0; i < 0x8000; i++)
			;
	}
}
/* EOF */
