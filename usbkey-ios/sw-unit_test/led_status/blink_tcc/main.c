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

void led_init(void);
void led_status(u32 mode);

static void clk32_test(void);
static void wait(u32 cycles);

/**
 * @brief Entry point and main loop of the unit-test
 *
 * Called by assembly bootstrap (see startup.s)
 */
int main(void)
{
	/* Initialize chip and low-level functions */
	hw_init();

	/* Initialize status LED */
	led_init();

	/* Enable 32kHz output on TP (for debug)*/
	clk32_test();

	/* Main test loop (infinite) */
	while (1) 
	{
		/* First blink example : ~ 2Hz, 1/2 duty cycle */
		led_status(0x00080010);
		wait(0x400000);
		
		/* Second blink example : 1Hz, 30ms on (short flash) */
		led_status(0x00010020);
		wait(0x400000);

		/* Third example : 8Hz, 1/2 duty cycle (fast blink) */
		led_status(0x00020004);
		wait(0x400000);
		
		/* LED OFF */
		led_status(0);
		wait(0x200000);
	}
}

/**
 * @brief Connect 32kHz GCLK to test-pin
 *
 * This function is used to connect internal 32kHz oscillator (through GCLK5)
 * tp test-point (P5). Mainly for debug :)
 */
void clk32_test(void)
{
	u32 v;

#ifdef XPLAINED
	/* Set PA21 as output */
	reg_wr(0x60000000 + 0x08, (1 << 21));
	/* PINCFG: enable PMUX to connect pin to GCLK */
	reg8_wr(0x60000000 + 0x55, 0x01);
	/* PINMUX */
	v  = reg8_rd(0x60000000 + 0x3A);
	v &= 0x0F;
	v |= (0x07 << 4);
	reg8_wr(0x60000000 + 0x3A, v);
#else
	/* Set PA11 as output */
	reg_wr(0x60000000 + 0x08, (1 << 11));
	/* PINCFG: enable PMUX to connect pin to GCLK */
	reg8_wr(0x60000000 + 0x4B, 0x01);
	/* PINMUX */
	v  = reg8_rd(0x60000000 + 0x35);
	v &= 0x0F;
	v |= (0x07 << 4);
	reg8_wr(0x60000000 + 0x35, v);
#endif
}

/**
 * @brief Initialize status LED
 *
 * This function must be called to initialize status LED and before any call to
 * led_status().
 */
void led_init(void)
{
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
