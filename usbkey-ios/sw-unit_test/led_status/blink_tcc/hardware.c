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
	u32 v;

	/* Update NVM (flash memory wait-state before any clock config) */
	reg_wr(NVM_ADDR + 0x04, (0 << 1)); /* Default: 0 = no wait state */

	/* Use PM to configure clock sources */
	reg8_wr(PM_ADDR + 0x08, 0x00); /* CPU  clock select (CPUSEL)  */
	reg8_wr(PM_ADDR + 0x09, 0x00); /* APBA clock select (APBASEL) */
	reg8_wr(PM_ADDR + 0x0A, 0x00); /* APBB clock select (APBBSEL) */
	reg8_wr(PM_ADDR + 0x0B, 0x00); /* APBC clock select (APBCSEL) */
	
	hw_init_clock();

#ifdef XPLAINED
        /* DIR: Set PB30 as output */
        reg_wr(0x60000080 + 0x08, (1 << 30));
	/* Set LED "OFF" (pin output=1) */
	reg_wr(0x60000080 + 0x18, (1 << 30));
        /* PINCFG: Configure PB30 (normal strength, no pull, no pmux) */
	reg8_wr(0x60000080 + 0x5E, 0x41);
	/* PINMUX: Configure alternate function E (TCC0) */
	v  = reg8_rd(0x60000080 + 0x3F);
	v &= 0xF0;
	v |= 0x04;
	reg8_wr(0x60000080 + 0x3F, v);
#else
        /* DIR: Set PA15 as output */
        reg_wr(0x60000000 + 0x08, (1 << 15));
	/* Set LED "OFF" (pin output=1) */
	reg_wr(0x60000000 + 0x18, (1 << 15));
        /* PINCFG: Configure PA15 (normal strength, no pull, use pmux) */
        reg8_wr(0x60000000 + 0x4F, 0x41);
	/* PINMUX: Configure alternate function F (TCC0) */
	v  = reg8_rd(0x60000000 + 0x37);
	v &= 0x0F;
	v |= (0x05 << 4);
	reg8_wr(0x60000000 + 0x37, v);
#endif
}

/**
 * @brief Configure oscillators and Generic Clock Controller (GCLK)
 *
 */
static inline void hw_init_clock(void)
{
	u32 v;

	/* Configure internal 8MHz oscillator */
	v = reg_rd(SYSCTRL_ADDR + 0x20); /* Read OSC8M config register */
	v &= 0xFFFFFC3F;                 /* Clear prescaler and OnDemand flag */
	reg_wr(SYSCTRL_ADDR + 0x20, v);  /* Write-back OSC8M */

	/* Wait for internal 8MHz oscillator stable and ready */
	while( ! (reg_rd(SYSCTRL_ADDR + 0x0C) & 0x08))
		;
	
	/* Activate the internal 32kHz oscillator */
	v  = (((reg_rd(0x00806024) >> 6) & 0x7F) << 16); /* Calib bits 38:44 */
	v |= (1 << 1); /* Set enable bit */
	v |= (1 << 2); /* Output Enable */
	reg_wr(SYSCTRL_ADDR + 0x18, v);
	/* Wait for internal 32kHz oscillator stable and ready */
	while( ! (reg_rd(SYSCTRL_ADDR + 0x0C) & 0x04))
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
	/* Set Divisor for GCLK5 : enabled, OSC32k, no divisor */
	reg_wr(GCLK_ADDR + 0x08, (1 << 8) | 0x05);
	reg_wr(GCLK_ADDR + 0x04, (1 << 19) | (1 << 16) | (0x04 << 8) | 0x05);
	/* Set Divisor for GCLK6 : disabled */
	reg_wr(GCLK_ADDR + 0x08, (1 << 8) | 0x06);
	reg_wr(GCLK_ADDR + 0x04, (0 << 16) | (0x06 << 8) | 0x06);
	/* Set Divisor for GCLK7 : disabled */
	reg_wr(GCLK_ADDR + 0x08, (1 << 8) | 0x07);
	reg_wr(GCLK_ADDR + 0x04, (0 << 16) | (0x06 << 8) | 0x07);
	/* Set Divisor for GCLK8 : disabled */
	reg_wr(GCLK_ADDR + 0x08, (1 << 8) | 0x08);
	reg_wr(GCLK_ADDR + 0x04, (0 << 16) | (0x06 << 8) | 0x08);
}
/* EOF */
