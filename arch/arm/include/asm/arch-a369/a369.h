/*
 * (C) Copyright 2012 Faraday Technology
 * Bing-Yao Luo <bjluo@faraday-tech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __A369_H
#define __A369_H

#include <asm/sizes.h>

#ifndef __ASSEMBLY__

#define REG32(off)	*(volatile unsigned long *)(off)
#define REG16(off)	*(volatile unsigned short *)(off)
#define REG8(off)	*(volatile unsigned char *)(off)

#endif /* __ASSEMBLY__ */

/*
 * Hardware register base
 */
#define CONFIG_FTUART010_BASE 	0x92B00000	/* UART controller */
#define CONFIG_FTPWMTMR010_BASE 0x92300000	/* Timer controller */
#define CONFIG_SCU_BASE 	0x92000000
#define CONFIG_FTWDT010_BASE	0x92200000	/* Watchdog */
#define CONFIG_FTGMAC100_BASE	0x90C00000	/* Ethernet Controller */

#endif	/* __A369_H */
