/**
 * @file  usb.c
 * @brief Stack for USB device
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
#include "types.h"
#include "usb.h"

static void _usb_load_calib(void);

ep_desc eps[8];

/**
 * @brief Initialize and start USB peripheral (device mode)
 *
 */
void usb_config(void)
{
	memset(eps, 0, sizeof(ep_desc) * 8);

	/* Wait end of a synchronization reset */
	while (reg8_rd(USB_ADDR + 0x02) & 0x01)
		;
	/* Initiate a software reset (SWRST) */
	reg8_wr(USB_ADDR + 0x00, reg8_rd(USB_ADDR + 0x00) | 0x02);
	/* Wait end of a synchronization reset */
	while (reg8_rd(USB_ADDR + 0x02) & 0x01)
		;

	_usb_load_calib();

	/* Set RUNSTDBY to keep clock active in standby mode */
	reg8_wr(USB_ADDR + 0x00, reg8_rd(USB_ADDR + 0x00) | (1 << 2));
	/* Set address of endpoints descriptors structure */
	reg_wr(USB_ADDR + 0x24, (u32)&eps);
	/* Set DETACH bit to avoid early USB connection */
	reg16_wr(USB_ADDR + 0x08, (0x00 << 2) | 1);
	
	/* ------------------------------------------------------------------ */

	/* Verify SYNCBUSY */
	if (reg8_rd(USB_ADDR + 0x02) & 0x03) {
		return; // Error -11
	}

	/* If the peripheral not already enabled */
	if ((reg8_rd(USB_ADDR + 0x00) & 0x02) == 0)
		/* Enable it ! */
		reg8_wr(USB_ADDR + 0x00, reg8_rd(USB_ADDR + 0x00) | 0x02);

	/* Enable USB interrupt into NVIC */
	reg_wr(0xE000E100, (1 << 7));

	/* Enable interrupts (RAMACER, EORST, SOF, SUSPEND) */
	reg16_wr(USB_ADDR + 0x18, 0x008D);

	/* Then, attach ----------------------------------------------------- */
	reg16_wr(USB_ADDR + 0x08, reg16_rd(USB_ADDR + 0x08) & 0xFFFE);
}

/**
 * @brief Initialize USB peripheral
 *
 */
void usb_init(void)
{
	/* Enable clocks for USB peripheral */
	reg_set(PM_ADDR + 0x1C, (1 << 5)); /* Set APB clock enable for USB */
	reg_set(PM_ADDR + 0x14, (1 << 6)); /* Set AHB clock enable for USB */
	/* Set and enable GCLK7 for USB */
	reg16_wr(GCLK_ADDR + 0x02, (1 << 14) | (0x07 << 8) | 0x06);

	/* 3) Configure IOs pins ----------------------- */

	/* PA24 (USB_DM) Set as output */
	reg_wr (0x60000000 + 0x08, (1 << 24));
	/* PA24 Set initial pin value to 0 */
	reg_wr (0x60000000 + 0x14, (1 << 24));
	/* PA24 Configure pin to use a peripheral (PMUX) */
	reg8_wr(0x60000000 + 0x58, 0x01);

	/* PA25 (USB_DP) Set as output */
	reg_wr(0x60000000 + 0x08, (1 << 25));
	/* PA25 Set initial pin value to 0 */
	reg_wr(0x60000000 + 0x14, (1 << 24));
	/* PA25 Configure pin to use a peripheral (PMUX) */
	reg8_wr(0x60000000 + 0x59, 0x01);

	/* Connect PA24 and PA25 to USB peripheral (PMUX) */
	reg8_wr(0x60000000 + 0x3C, (0x06 << 4) | (0x06 << 0));
}

/**
 * @brief Interrupt handler
 *
 */
void usb_irq(void)
{
	u32 status;
	u16 epint = reg16_rd(USB_ADDR + 0x20);

	/* Read interrupt status flags */
	status  = reg16_rd(USB_ADDR + 0x1C);
	/* Keep only enable bits */
	status &= reg16_rd(USB_ADDR + 0x18);

	/* If Start-Of-Frame interrupt is set */
	if (status & (1 << 2))
	{
		/* Ack/clear event */
		reg16_wr(USB_ADDR + 0x1C, (1 << 2));
	}
	/* EORST */
	if (status & (1 << 3))
	{
		/* Ack/clear this event */
		reg16_wr(USB_ADDR + 0x1C, (1 << 3));
		/* Enable Suspend interrupt */
		reg16_wr(USB_ADDR + 0x18, (1 << 0));
	}
	/* If the WAKEUP bit is set */
	if (status & (1 << 4))
	{
		/* Disable WAKEUP interrupt */
		reg16_wr(USB_ADDR + 0x14, (1 << 4));
		/* Ack/clear event */
		reg16_wr(USB_ADDR + 0x1C, (1 << 4));
		/* Enable SUSPEND interrupt */
		reg16_wr(USB_ADDR + 0x18, (1 << 0));
	}
	/* If the SUSPEND bit is set */
	if (status & (1 << 0))
	{
		/* Disable SUSPEND interrupt */
		reg16_wr(USB_ADDR + 0x14, (1 << 0));
		/* Ack/clear event */
		reg16_wr(USB_ADDR + 0x1C, (1 << 0));
		/* Enable WAKEUP interrupt */
		reg16_wr(USB_ADDR + 0x18, (1 << 4));
	}
	/* If RAMACR bit is set */
	if (status & (1 << 7))
	{
		/* Ack/clear event */
		reg16_wr(USB_ADDR + 0x1C, (1 << 7));
	}
}

/**
 * @brief Load USB calibration values from NVM
 *
 */
static void _usb_load_calib(void)
{
#define NVM_AUX1_4 0x00806020
#define NVM_USB_PAD_TRANSN_POS  45
#define NVM_USB_PAD_TRANSN_SIZE  5
#define NVM_USB_PAD_TRANSP_POS  50
#define NVM_USB_PAD_TRANSP_SIZE  5
#define NVM_USB_PAD_TRIM_POS    55
#define NVM_USB_PAD_TRIM_SIZE    3
	u32 pad_transn
	    = (*((u32 *)(NVM_AUX1_4) + (NVM_USB_PAD_TRANSN_POS / 32)) >> (NVM_USB_PAD_TRANSN_POS % 32))
	      & ((1 << NVM_USB_PAD_TRANSN_SIZE) - 1);
	u32 pad_transp
	    = (*((u32 *)(NVM_AUX1_4) + (NVM_USB_PAD_TRANSP_POS / 32)) >> (NVM_USB_PAD_TRANSP_POS % 32))
	      & ((1 << NVM_USB_PAD_TRANSP_SIZE) - 1);
	u32 pad_trim = (*((u32 *)(NVM_AUX1_4) + (NVM_USB_PAD_TRIM_POS / 32)) >> (NVM_USB_PAD_TRIM_POS % 32))
	                    & ((1 << NVM_USB_PAD_TRIM_SIZE) - 1);
	if (pad_transn == 0x1F) {
		pad_transn = 5;
	}
	if (pad_transp == 0x1F) {
		pad_transp = 29;
	}
	if (pad_trim == 0x7) {
		pad_trim = 5;
	}

	reg16_wr(USB_ADDR + 0x28, (pad_trim << 12) |(pad_transn << 6) | (pad_transp << 0));

	reg8_wr(USB_ADDR + 0x03, (3 << 0) | (3 << 2));
}
/* EOF */
