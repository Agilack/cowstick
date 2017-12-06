/**
 * @file  flash.c
 * @brief Functions to erase/read/write internal flash memory
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
#include "flash.h"
#include "hardware.h"

/**
 * @brief Erase one page of memory
 *
 * @param addr Start address of the page to erase
 */
int flash_erase(u32 addr)
{
	/* Set ADDR */
	reg_wr(NVM_ADDR + 0x1C, (addr / 2));
	/* Erase Row command */
	reg16_wr(NVM_ADDR + 0x00, (0xA5 << 8) | 0x02);
	/* Wait for command to complete */
	while( (reg8_rd(NVM_ADDR+0x14) & 1) == 0)
		;

	if (reg8_rd(NVM_ADDR+0x14) & 0x02)
	{
		u16 status;
		/* Read (and return) status bits */
		status = reg16_rd(NVM_ADDR+0x18);
		return(status);
	}

	return(0);
}

/**
 * @brief Write one page of memory
 *
 * @param addr Start address of the datas to write
 * @param data Pointer to the datas (source)
 */
void flash_write(u32 addr, u8 *data)
{
	u32 *pdest;
	int len;
	int  i;

	len = 64;
	pdest = (u32 *)addr;
	for (i = 0; i < 16; i++)
	{
		u32 word;
		
		word  = (data[3] << 24) | (data[2] << 16);
		word |= (data[1] <<  8) | (data[0] <<  0);
		pdest[i] = word;
		len  -= 4;
		data += 4;
		if (len <= 0)
			break;
	}
	/* Set ADDR */
	reg_wr(NVM_ADDR + 0x1C, addr);
	/* Write page command */
	reg16_wr(NVM_ADDR + 0x00, (0xA5 << 8) | 0x04);
	while( (reg8_rd(NVM_ADDR+0x14) & 1) == 0)
		;
	return;
}
/* EOF */
