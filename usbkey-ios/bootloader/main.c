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

/**
 * @brief Entry point of the bootloader
 *
 */
int main(void)
{
	/* Initialize low-level hardware*/
	hw_init();
	
	/* Infinite loop, do nothing */
	while(1)
		;

	/* Hey, this is only a skeleton :) */
	return(0);
}

/* EOF */
