/**
 * @file  flash.h
 * @brief Definitions and function prototypes for internal flash-memory access
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
#ifndef FLASH_H
#define FLASH_H

#include "types.h"

int  flash_erase(u32 addr);
void flash_write(u32 addr, u8 *data);

#endif
/* EOF */
