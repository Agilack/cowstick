/**
 * @file  libc.c
 * @brief Standard library functions
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
#include "libc.h"
#include "types.h"

/**
 * @brief Copy 'n' bytes from a source buffer to a destination buffer
 *
 * @param dst Pointer to the destination buffer
 * @param src Pointer to the source buffer
 * @param n   NUmber of bytes to copy
 */
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

/**
 * @brief Fill a buffer with a value (set all bytes)
 *
 * @param dst   Pointer to the buffer to fill
 * @param value Value to set for each bytes
 * @param n     Number of bytes to fill
 */
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

/****************************** STRING functions ******************************/
#ifdef USE_LIBC_STRING

/**
 * @brief Copy a string to another buffer
 *
 * @param dest Pointer to the destination buffer
 * @param src  Pointer to the string to copy (source). Must be NULL terminated
 * @return Pointer to the destination buffer
 */
char *strcpy(char *dest, const char *src)
{
	return strncpy(dest, src, strlen(src));
}

/**
 * @brief Count the number of character into a string
 *
 * @param Pointer to the string (NULL terminated)
 * @return Number of character into the string
 */
int strlen(const char *str)
{
        int count;
        count = 0;
        while(*str)
        {
                count ++;
                str++;
        }
        return(count);
}

/**
 * @brief Copy a string to another buffer, with a limit of 'len' bytes
 *
 * @param dest Pointer to the destination buffer
 * @param src  Pointer to the string to copy (source). Must be NULL terminated
 * @param len  Maximum number of character to copy
 * @return Number of character into the string
 */
char *strncpy(char *dest, const char *src, int len)
{
        char *dsave = dest;
        while ( (*src) && (len > 0))
        {
                *dest = *src;
                src++;
                dest++;
                len--;
        }
        *dest = 0;
        return dsave;
}
#endif

/* EOF */
