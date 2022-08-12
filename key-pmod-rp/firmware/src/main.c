/**
 * @file  main.c
 * @brief Entry point and main function of the firmware
 *
 * @author Saint-Genest Gwenael <gwen@cowlab.fr>
 * @copyright Cowlab (c) 2022
 *
 * @page License
 * This firmware is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3 as published
 * by the Free Software Foundation. You should have received a copy of the
 * GNU General Public License along with this program, see LICENSE.md file
 * for more details.
 * This program is distributed WITHOUT ANY WARRANTY.
 */
#include "pico/stdlib.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "serial.h"

#undef USE_ECHO

void u_process(void);

/**
 * @brief Entry point of C code
 *
 * This function is called by low-level reset handler. This can be after a
 * power-on, an hardware or software reboot, and after some critical errors.
 * This function should never returns.
 */
int main()
{
	stdio_init_all();

	/* Wait a bit after start to allow VCP to resync */
	sleep_ms(2000);
	printf("--=={ Cowstick-RP }==--\n");

	serial_init();

	while(1)
	{
		if (serial_rx_avail() > 0)
			u_process();
	}
}

void u_process(void)
{
	char data[64];
	char msg[80];
	int len;

	memset(data, 0, 64);
	/* Get data received from uart */
	serial_read(data, 63);
	/* Write debug message to standard output */
	sprintf(msg, "[%s]\r\n", data);
	printf("%s", data);
#ifdef USE_ECHO
	/* Send debug message to uart itself as echo */
	serial_write(msg, strlen(msg));
#endif
}
/* EOF */
