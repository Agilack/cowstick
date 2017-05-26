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
void usb_ep_enable(u8 ep, u8 mode, usb_module *mod);
void usb_reset(usb_module *mod);
static void ep_irq(usb_module *mod, u8 ep);
static void ep_transfer_in(usb_module *mod, u8 ep, int isr);
static void ep_transfer_out  (usb_module *mod, u8 ep, int isr);
static void ep_transfer_setup(usb_module *mod, u8 ep);
void usb_trans_setup(usb_module *mod, u8 ep);
static void std_get_descriptor(usb_module *mod);

/**
 * @brief Initialize and start USB peripheral (device mode)
 *
 */
void usb_config(usb_module *mod)
{
	int i;

	for (i = 0; i < 8; i++)
	{
		mod->ep_status[i].size  = 0;
		mod->ep_status[i].count = 0;
		mod->ep_status[i].flags = 0;
	}

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
	reg_wr(USB_ADDR + 0x24, (u32)&mod->ep_desc);
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
 * @param mod Pointer to the USB module configuration
 */
void usb_irq(usb_module *mod)
{
	u32 status;
	u16 epint = reg16_rd(USB_ADDR + 0x20);

	/* Read interrupt status flags */
	status  = reg16_rd(USB_ADDR + 0x1C);
	/* Keep only enable bits */
	status &= reg16_rd(USB_ADDR + 0x18);

	if (status)
	{
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

			/* Bus has been reset, reset module and status */
			usb_reset(mod);
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
	else if (epint)
	{
		ep_irq(mod, 0);
	}
}

/**
 * @brief Reset state of module after a bus reset
 *
 * @param mod Pointer to the USB module configuration
 */
void usb_reset(usb_module *mod)
{
	/* Reset EP0 type */
	reg8_wr(USB_ADDR + 0x100, 0x00);

	/* Ack/clear EORST event */
	reg16_wr(USB_ADDR + 0x1C, (1 << 3));
	/* Disable WAKEUP interrupt */
	reg16_wr(USB_ADDR + 0x14, (1 << 4));
	/* Enable SUSPEND interrupt */
	reg16_wr(USB_ADDR + 0x18, (1 << 0));

	/* Clear EP descriptors config */
	memset(&mod->ep_desc, 0, sizeof(ep_desc) * 8);

	mod->addr = 0;

	usb_ep_enable(0, 0x11, mod);
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

void usb_ep_enable(u8 ep, u8 mode, usb_module *mod)
{
	reg8_wr(USB_ADDR + 0x100 + (ep * 0x20), mode);

	mod->ep_desc[ep].b0_pcksize = 0x30100000;
	mod->ep_desc[ep].b1_pcksize = 0x30000040;

	mod->ep_status[ep].flags = 0;
	mod->ep_status[ep].size = 0;

	/* Disable bank 0 (set BK0RDY)   */
	reg8_wr(USB_ADDR + 0x105, (1 << 6));
	/* Disable bank 1 (clear BK1RDY) */
	reg8_wr(USB_ADDR + 0x104, (1 << 7));

	usb_trans_setup(mod, ep);
}

static void ep_irq(usb_module *mod, u8 ep)
{
	u8 flags = reg8_rd(USB_ADDR + 0x107);

	/* If the busy flag is not set */
	if ((mod->ep_status[ep].flags & 1) == 0)
	{
		/* ==== Setup transaction ==== */

		if (flags & (1 << 4))
			ep_transfer_setup(mod, 0);
		/* If STALL1 bit is set */
		else if (flags & (1 << 6))
		{
			// ToDo
		}
		/* If STALL0 bit is set */
		else if (flags & (1 << 5))
		{
			// ToDo
		}

	}
	/* If the DIR flag is set : IN */
	else if (mod->ep_status[ep].flags & 2)
	{
		/* If Tranfser Stall on bank 1 (TRSTALL1) */
		if (flags & (1 << 6))
		{
			/* Ack/clear event */
			reg8_wr(USB_ADDR + 0x107, (1 << 6));
		}
		/* If Tranfser Fail on bank 1 (TRFAIL1) */
		else if (flags & (1 << 3))
		{
			/* Clear Bank1 status */
			mod->ep_desc[ep].b1_status_bk = 0;
			/* Ack/Clear the event into flags */
			reg8_wr(USB_ADDR + 0x107, (1 << 3));
			/* Disable this interrupt */
			reg8_wr(USB_ADDR + 0x108, (1 << 3));
		}
		/* If Transfer Complete on bank 1 (TRCPT1) */
		else if (flags & (1 << 1))
		{
			// ToDo: Send next data packet (if any)
			ep_transfer_in(mod, ep, 1);

			//if (ctrl)
			//reg8_wr(USB_ADDR + 0x108, 0x4A | 0x01);
			//else
			//reg8_wr(USB_ADDR + 0x108, 0x4A);

			/* Ack/clear the TRCPT interrupt */
			reg8_wr(USB_ADDR + 0x107 + (ep * 0x20), (1 << 1));
		}
//		else if (ctrl)
//		{
//			//
//		}
	}
	/* Else, OUT */
	else
	{
		/* In case of a STALL event (STALL0) */
		if (flags & (1 << 5))
		{
			/* Ack/clear the TRCPT interrupt */
			reg8_wr(USB_ADDR + 0x107 + (ep * 0x20), (1 << 5));
		}
		/* If Transfer Complete on bank 0 (TRCPT0) */
		if (flags & (1 << 0))
		{
			ep_transfer_out(mod, ep, 1);
			/* Ack/clear the TRCPT interrupt */
			reg8_wr(USB_ADDR + 0x107 + (ep * 0x20), (1 << 0));
		}
		/* If a Tranfer Fail has been detected (TRFAIL) */
		else if (flags & (1 << 2))
		{
			/* Clear Bank0 status */
			mod->ep_desc[ep].b0_status_bk = 0;
			/* Disable this interrupt */
			reg8_wr(USB_ADDR + 0x108, (1 << 2));
			/* Ack/Clear the event into flags */
			reg8_wr(USB_ADDR + 0x107, (1 << 2));
		}
	}
}

static void ep_transfer_complete(usb_module *mod, u8 ep)
{
	int dir;

	dir = (mod->ep_status[ep].flags & 0x02);

	mod->ep_status[ep].flags &= ~(0x02 | EP_ZLP | EP_BUSY);

	if (mod->ep_status[ep].flags & 0x10)
	{
		mod->ep_status[ep].flags &= ~0x10;
		if (mod->addr != 0)
			/* Set device address */
			reg8_wr(USB_ADDR + 0x0A, (1 << 7) | mod->addr);
	}
	else
	{
		if (dir)
		{
			mod->ep_status[ep].flags |=  EP_BUSY;
			mod->ep_status[ep].flags &=  ~2;
			mod->ep_status[ep].flags |=  EP_ZLP;
			mod->ep_status[ep].size  = 0;
			mod->ep_status[ep].count = 0;
			ep_transfer_out(mod, ep, 0);
		}
	}
}

/**
 * @brief Initiate (or continue) a IN transfer data phase
 *
 * @param mod Pointer to the USB module configuration
 * @param ep  Endpoint number
 * @param isr True when called by an interrupt
 */
static void ep_transfer_in(usb_module *mod, u8 ep, int isr)
{
	int len   = mod->ep_status[ep].size;
	int count = 0;

	if (isr)
		count = mod->ep_desc[ep].b1_pcksize;

	// ToDO : ACK TRCPT0 if called by ISR

	/* Update the number of processed bytes */
	mod->ep_status[ep].count += count;

	if (mod->ep_status[ep].count < mod->ep_status[ep].size)
	{
		/* Set buffer address */
		mod->ep_desc[ep].b1_addr = (u32)&mod->ctrl_in;
		/* Set buffer length */
		mod->ep_desc[ep].b1_pcksize = 0x30000000 | len;

		/* Set All interrupts */
		reg8_wr(USB_ADDR + 0x109, 0x35);

		/* Set BK1RDY */
		reg8_wr(USB_ADDR + 0x105, (1 << 7));
	}
	/* Zero Length Packet */
	else if (mod->ep_status[ep].flags & EP_ZLP)
	{
		/* Clear ZLP flag after use it */
		mod->ep_status[ep].flags &= ~EP_ZLP;
		/* Set buffer address */
		mod->ep_desc[ep].b1_addr = (u32)&mod->ctrl_in;
		/* Set buffer length */
		mod->ep_desc[ep].b1_pcksize = 0x30000000;

		/* Set All interrupts */
		reg8_wr(USB_ADDR + 0x109, 0x4A);

		/* Set BK1RDY */
		reg8_wr(USB_ADDR + 0x105, (1 << 7));
	}
	else
	{
		/* Disable all Bank1 interrupts */
		reg8_wr(USB_ADDR + 0x108, 0x4A);

		ep_transfer_complete(mod, ep);
	}
}

/**
 * @brief Initiate (or continue) a OUT transfer data phase
 *
 * @param mod Pointer to the USB module configuration
 * @param ep  Endpoint number
 */
static void ep_transfer_out(usb_module *mod, u8 ep, int isr)
{
	int len = mod->ep_status[ep].size;

	if (isr)
		mod->ep_desc[ep].b0_status_bk = 0;

	if (len > 0)
	{
		u32 pcksize = 0x30000000 | (0x40 << 14) | (0 << 0);
		/* Set buffer length : 0 (ZLP) */
		mod->ep_desc[ep].b0_pcksize = pcksize;
		/* Set All Bank0 interrupts */
		reg8_wr(USB_ADDR + 0x109, 0x35);
		/* Clear BK0RDY */
		reg8_wr(USB_ADDR + 0x104, (1 << 6));
	}
	else if (mod->ep_status[ep].flags & EP_ZLP)
	{
		mod->ep_status[ep].flags &= ~EP_ZLP;
		/* Set buffer length : 0 (ZLP) */
		mod->ep_desc[ep].b0_pcksize = 0x30000000 | (0x40 << 14);
		/* Set All Bank0 interrupts */
		reg8_wr(USB_ADDR + 0x109, 0x35);
		/* Clear BK0RDY */
		reg8_wr(USB_ADDR + 0x104, (1 << 6));
		mod->ep_status[ep].flags &= ~EP_BUSY;
	}
	else
	{
		/* Disable all Bank0 interrupts */
		reg8_wr(USB_ADDR + 0x108, 0x35);

		ep_transfer_complete(mod, ep);
	}
}

/**
 * @brief Handle a transfer in SETUP phase
 *
 * @param mod Pointer to the USB module configuration
 * @param ep  Endpoint number
 */
static void ep_transfer_setup(usb_module *mod, u8 ep)
{
	int i;
	u16 bytes = (mod->ep_desc[ep].b0_pcksize & 0x3FF);

	/* Clear bank status */
	mod->ep_desc[ep].b0_status_bk = 0;
	mod->ep_desc[ep].b1_status_bk = 0;
	/* Ack event (all bank 0 and all bank 1) */
	reg8_wr(USB_ADDR + 0x107 + (ep * 0x20), 0x4A | 0x25);
	/* Disable transfer interrupts (all bank 0 and all bank 1) */
	reg8_wr(USB_ADDR + 0x108 + (ep * 0x20), 0x4A | 0x25);

	/* Callback */
	if (bytes > 0)
	{
		/* GET_DESCRIPTOR */
		if ((mod->ctrl[0] == 0x80) && (mod->ctrl[1] == 0x06))
		{
			std_get_descriptor(mod);
			/* Ack/clear the RXSTP interrupt */
			reg8_wr(USB_ADDR + 0x107 + (ep * 0x20), (1<< 4));
		}
		/* SET_ADDRESS */
		else if ((mod->ctrl[0] == 0x00) && (mod->ctrl[1] == 0x05))
		{
			/* Save device address */
			mod->addr = mod->ctrl[2];

			memset(mod->ctrl_in, 0, 64);
			mod->ep_status[ep].count  = 0;
			mod->ep_status[ep].size = 0x00;
			mod->ep_status[ep].flags |=  EP_BUSY;
			mod->ep_status[ep].flags |=  2;   // Set dir IN
			mod->ep_status[ep].flags |=  EP_ZLP;
			mod->ep_status[ep].flags |= 0x10; // Debug flag
			ep_transfer_in(mod, ep, 0);

			/* Ack/clear the RXSTP interrupt */
			reg8_wr(USB_ADDR + 0x107 + (ep * 0x20), (1<< 4));
		}
	}
}

void usb_trans_setup(usb_module *mod, u8 ep)
{
	mod->ep_desc[ep].b0_addr = (u32)mod->ctrl;
	mod->ep_desc[ep].b0_pcksize = 0x30000000 | (0x40 << 14) | (0 << 0);

	/* Clear EP Status (BK1RDY and STALLRQx) */
	reg8_wr(USB_ADDR + 0x104 + (ep * 20), (1 << 7) | (0x03 << 4));
	/* Set BK0 */
	reg8_wr(USB_ADDR + 0x105 + (ep * 20), (1 << 6));

	/* Enable RXSTP interrupt */
	reg8_wr(USB_ADDR + 0x109 + (ep * 20), (1 << 4));
}

static void std_get_descriptor(usb_module *mod)
{
	/* Device Descriptor */
	if (mod->ctrl[3] == 1)
	{
		/* Copy descriptor into ep buffer */
		memcpy(mod->ctrl_in, mod->desc, 0x12);
		/* Configure and start transfer */
		mod->ep_status[0].count  = 0;
		mod->ep_status[0].size   = 0x12;
		mod->ep_status[0].flags |= EP_BUSY;
		mod->ep_status[0].flags |= 2; // Set dir IN
		ep_transfer_in(mod, 0, 0);
	}
	/* Configuration descriptor */
	else if (mod->ctrl[3] == 2)
	{
		u8 desc[] = { 0x09, 0x02,
			34, 0x00,   /* wTotalLength        */
			0x01,       /* bNumInterfaces      */
			0x01,       /* bConfigurationValue */
			0x00,       /* iConfiguration      */
			0x80,       /* bmAttributes : Bus Powered */
			0x32};      /* MaxPower : 100mA    */

		/* Copy descriptor into ep buffer */
		memcpy(mod->ctrl_in, desc, 0x09);
		/* Configure and start transfer */
		mod->ep_status[0].count  = 0;
		mod->ep_status[0].size   = 0x09;
		mod->ep_status[0].flags |= EP_BUSY;
		mod->ep_status[0].flags |= 2; // Set dir IN
		ep_transfer_in(mod, 0, 0);
	}
}

/* EOF */
