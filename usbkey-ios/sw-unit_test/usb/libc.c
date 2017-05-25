/**
 * @file  libc.c
 * @brief Standard library functions
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
#include "libc.h"
#include "types.h"

void *memcpy(void *dst, const void *src, int n)
{
        u8 *s;
        u8 *d;
        s = (u8*)src;
        d = (u8*)dst;
        while(n)
        {
                *d = *s;
                s ++;
                d ++;
                n --;
        }
        return(dst);
}

void *memset(void *dst, int value, int n)
{
        u8 *d;
        d = (u8 *)dst;
        while(n)
        {
                *d  = value;
                d++;
                n --;
        }
        return(dst);
}
/* EOF */
