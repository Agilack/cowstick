/**
 * @file  hardware.c
 * @brief Low-level hardware configuration
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

static inline void hw_init_clock(void);

/**
 * @brief Called on startup to init processor, clocks and some peripherals
 *
 */
void hw_init(void)
{
	/* Update NVM (flash memory wait-state before any clock config */
	reg_wr(NVM_ADDR + 0x04, (0 << 1)); /* Default: 0 = no wait state */

	/* Use PM to configure clock sources */
	reg8_wr(PM_ADDR + 0x08, 0x00); /* CPU  clock select (CPUSEL)  */
	reg8_wr(PM_ADDR + 0x09, 0x00); /* APBA clock select (APBASEL) */
	reg8_wr(PM_ADDR + 0x0A, 0x00); /* APBB clock select (APBBSEL) */
	reg8_wr(PM_ADDR + 0x0B, 0x00); /* APBC clock select (APBCSEL) */
	
	hw_init_clock();
}

/**
 * @brief Configure oscillators and Generic Clock Controller (GCLK)
 *
 */
static inline void hw_init_clock(void)
{
	u32 dfll_coarse, dfll_cfg;
	u32 v;

	/* Configure internal 8MHz oscillator */
	v = reg_rd(SYSCTRL_ADDR + 0x20); /* Read OSC8M config register */
	v &= 0xFFFFFC3F;                 /* Clear prescaler and OnDemand flag */
	reg_wr(SYSCTRL_ADDR + 0x20, v);  /* Write-back OSC8M */
	/* Wait for internal 8MHz oscillator stable and ready */
	while( ! (reg_rd(SYSCTRL_ADDR + 0x0C) & 0x08))
		;

	/* Set Divisor for GCLK0 : enabled, OSC8M, no divisor */
	reg_wr(GCLK_ADDR + 0x08, (1 << 8) | 0x00);
	reg_wr(GCLK_ADDR + 0x04, (1 << 16) | (0x06 << 8) | 0x00);
	/* Set Divisor for GCLK1 : disabled */
	reg_wr(GCLK_ADDR + 0x08, (1 << 8) | 0x01);
	reg_wr(GCLK_ADDR + 0x04, (0 << 16) | (0x06 << 8) | 0x01);
	/* Set Divisor for GCLK2 : disabled */
	reg_wr(GCLK_ADDR + 0x08, (1 << 8) | 0x02);
	reg_wr(GCLK_ADDR + 0x04, (0 << 16) | (0x06 << 8) | 0x02);
	/* Set Divisor for GCLK3 : disabled */
	reg_wr(GCLK_ADDR + 0x08, (1 << 8) | 0x03);
	reg_wr(GCLK_ADDR + 0x04, (0 << 16) | (0x06 << 8) | 0x03);
	/* Set Divisor for GCLK4 : disabled */
	reg_wr(GCLK_ADDR + 0x08, (1 << 8) | 0x04);
	reg_wr(GCLK_ADDR + 0x04, (0 << 16) | (0x06 << 8) | 0x04);
	/* Set Divisor for GCLK5 : disabled */
	reg_wr(GCLK_ADDR + 0x08, (1 << 8) | 0x05);
	reg_wr(GCLK_ADDR + 0x04, (0 << 16) | (0x06 << 8) | 0x05);
	/* Set Divisor for GCLK6 : disabled */
	reg_wr(GCLK_ADDR + 0x08, (1 << 8) | 0x06);
	reg_wr(GCLK_ADDR + 0x04, (0 << 16) | (0x06 << 8) | 0x06);
	/* Set Divisor for GCLK7 : enabled, DFLL48M, no divisor */
	reg_wr(GCLK_ADDR + 0x08, (1 << 8) | 0x07);
	reg_wr(GCLK_ADDR + 0x04, (1 << 16) | (0x07 << 8) | 0x07);
	/* Set Divisor for GCLK8 : disabled */
	reg_wr(GCLK_ADDR + 0x08, (1 << 8) | 0x08);
	reg_wr(GCLK_ADDR + 0x04, (0 << 16) | (0x06 << 8) | 0x08);

	/* Enable DFLL block */
	reg16_wr(SYSCTRL_ADDR + 0x24, (1 << 1));
	while ( ! (reg_rd(SYSCTRL_ADDR + 0x0C) & 0x00000010))
		;
	/* Configure DFLL multiplier (DFLLMUL) */
	reg_wr(SYSCTRL_ADDR + 0x2c, (1 << 20) | (1 << 16) | 0xBB80);
	/* Set DFLL cabration value */
	dfll_coarse = (*(u32 *)0x00806024 >> 26) & 0x3F;
	reg_wr(SYSCTRL_ADDR + 0x28, (0x0000 << 16) | (dfll_coarse << 10) | 512);
	/* Configure DFLL */
	//dfll_cfg = (1<<10)|(1<<8)|(1<<5)|(1<<2)|(1<<1);
	dfll_cfg = (1<<10)|(1<<8)|(1<<5)|(1<<1);
	reg16_wr(SYSCTRL_ADDR + 0x24, dfll_cfg);

	/* Wait depending on DFLL mode (closed or open loop) */
	if (reg_rd(SYSCTRL_ADDR + 0x24) & 0x04)
	{
		u32 wait_mask = 0x10;
		/* Wait for DFLL to be ready */
		while((reg_rd(SYSCTRL_ADDR + 0X0C) & wait_mask) != wait_mask)
			;
	}
	else
	{
		/* Wait for DFLL to be ready */
		while((reg_rd(SYSCTRL_ADDR + 0X0C) & 0x10) == 0)
			;
	}

	/* Wait end of clock domains synchronization */
	while (reg8_rd(GCLK_ADDR + 0x01) & 0x80)
		;

	/* Disable after all possible configurations needs sync written. */
	reg_wr(SYSCTRL_ADDR + 0x18, reg_rd(SYSCTRL_ADDR + 0x18) & ~0x02);
}
/* EOF */
