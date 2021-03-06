/**
 * @file  libc.h
 * @brief Definitions and prototypes for standard library functions
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
#ifndef LIBC_H
#define LIBC_H

void *memcpy (void *dst, const void *src, int n);
void *memset (void *dst, int value, int n);
#ifdef USE_LIBC_STRING
char *strcpy (char *dest, const char *src);
int   strlen (const char *str);
char *strncpy(char *dest, const char *src, int len);
#endif

#endif
