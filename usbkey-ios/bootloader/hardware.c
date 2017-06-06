/**
 * @file  hardware.c
 * @brief Low-level hardware configuration
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

static inline void hw_init_button(void);
static inline void hw_init_clock(void);
static inline void hw_init_leds(void);

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

	hw_init_button();
	hw_init_leds();
}

/**
 * @brief Initialize io for pushbutton
 *
 */
static inline void hw_init_button(void)
{
#ifdef XPLAINED
	/* DIR : set PA15 as input */
	reg_wr (0x60000000 + 0x04, (1 << 15));
	/* Configure pull-up (set data out to '1') */
	reg_wr (0x60000000 + 0x18, (1 << 15));
	/* PINCFG : enable input driver and activate pull resistor */
	reg8_wr(0x60000000 + 0x4F, (1 << 2) | (1 << 1));
	/* Enable input sampling */
	reg_set(0x60000000 + 0x24, (1 << 15));
#else
	/* DIR : set PA02 as input */
	reg_wr (0x60000000 + 0x04, (1 << 2));
	/* Configure pull-up (set data out to '1') */
	reg_wr (0x60000000 + 0x18, (1 << 2));
	/* PINCFG : enable input driver and activate pull resistor */
	reg8_wr(0x60000000 + 0x42, (1 << 2) | (1 << 1));
	/* Enable input sampling */
	reg_set(0x60000000 + 0x24, (1 << 2));
#endif
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
	reg_wr(GCLK_ADDR + 0x04, (1 << 16) | (0x04 << 8) | 0x05);
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

/**
 * @brief Initialize status LED
 *
 * This function must be called to initialize status LED and before any call to
 * led_status().
 */
static inline void hw_init_leds(void)
{
	u32 v;
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

	/* Enable TCC0 clock (APBCMASK) */
	reg_set(PM_ADDR + 0x20, (1 << 8));
	/* Set GCLK for TCC0 (generic clock generator 5) */
	reg16_wr (GCLK_ADDR + 0x02, (1 << 14) | (5 << 8) | 0x1A);
	/* Reset TCC0 peripheral */
	reg_wr(TCC0_ADDR + 0x00, (1 << 0));
	while (reg_rd(TCC0_ADDR + 0x00) & 0x01)
		;
	/* Waveform */
	reg_wr(TCC0_ADDR + 0x3C, (0x02 << 0));
#ifdef XPLAINED
	/* Invert output */
	reg_wr(TCC0_ADDR + 0x18, (0x01 << 16));
#endif
	/* Initial value : LED OFF */
	led_status(0);
	/* Enable TCC0 */
	reg_wr(TCC0_ADDR + 0x00, (1 << 1));
}

/**
 * @brief Get status of push-button
 *
 * @return boolean True if the button is pushed
 */
int button_status(void)
{
	u32 v;
#ifdef XPLAINED
	v = reg_rd(0x60000000 + 0x20);
	if ((v & (1 << 15)) == 0)
		return(1);
#else
	v = reg_rd(0x60000000 + 0x20);
	if ((v & (1 << 2)) == 0)
		return(1);
#endif
	return(0);
}

/**
 * @brief Set status LED state
 *
 * @param mode New blink freq and duty cycle (or 1/0 for on/off)
 */
void led_status(u32 mode)
{
	/* When mode 0 is requested : LED off */
	if (mode < 2)
	{
#ifdef XPLAINED
		/* PINCFG: Configure pin as GPIO */
		reg8_wr(0x60000080 + 0x5E, 0x40);
		/* Set GPIO 1 or 0 depending on mode value */
		if (mode == 0)
			reg_wr(0x60000080 + 0x18, (1 << 30));
		else
			reg_wr(0x60000080 + 0x14, (1 << 30));
#else
		/* PINCFG: Configure pin as GPIO */
		reg8_wr(0x60000000 + 0x4F, 0x40);
		/* Set GPIO 1 or 0 depending on mode value */
		if (mode == 0)
			reg_wr(0x60000000 + 0x14, (1 << 15));
		else
			reg_wr(0x60000000 + 0x18, (1 << 15));
#endif
	}
	else
	{
		u32 period = (mode & 0xFFFF);
		u32 toggle = (mode >> 16);

		/* Set period length (top defined by PER) */
		reg_wr(TCC0_ADDR + 0x40, (period << 10));
#ifdef XPLAINED
		/* PINCFG: Configure pin to use TCC */
		reg8_wr(0x60000080 + 0x5E, 0x41);
		/* Set Toggle value (CC0) */
		reg_wr(TCC0_ADDR + 0x44, (toggle << 10));
#else
		/* PINCFG: Configure pin to use TCC */
		reg8_wr(0x60000000 + 0x4F, 0x41);
		/* Set Toggle value (CC1) */
		reg_wr(TCC0_ADDR + 0x48, (toggle << 10));
#endif
		/* Reset COUNT */
		reg_wr(TCC0_ADDR + 0x34, 1);
	}
}
/* EOF */
