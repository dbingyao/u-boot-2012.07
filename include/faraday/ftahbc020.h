/*
 * (C) Copyright 2009 Faraday Technology
 * Po-Yu Chuang <ratbert@faraday-tech.com>
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

/*
 * AHB Controller
 */
#ifndef __FTAHBC020_H
#define __FTAHBC020_H

#define FTAHBC020_OFFSET_SLAVE0_BSR	0x00
#define FTAHBC020_OFFSET_SLAVE1_BSR	0x04
#define FTAHBC020_OFFSET_SLAVE2_BSR	0x08
#define FTAHBC020_OFFSET_SLAVE3_BSR	0x0C
#define FTAHBC020_OFFSET_SLAVE4_BSR	0x10
#define FTAHBC020_OFFSET_SLAVE5_BSR	0x14
#define FTAHBC020_OFFSET_SLAVE6_BSR	0x18
#define FTAHBC020_OFFSET_SLAVE7_BSR	0x1C
#define FTAHBC020_OFFSET_SLAVE9_BSR	0x24
#define FTAHBC020_OFFSET_SLAVE12_BSR	0x30
#define FTAHBC020_OFFSET_SLAVE13_BSR	0x34
#define FTAHBC020_OFFSET_SLAVE14_BSR	0x38
#define FTAHBC020_OFFSET_SLAVE15_BSR	0x3C
#define FTAHBC020_OFFSET_SLAVE17_BSR	0x44
#define FTAHBC020_OFFSET_SLAVE18_BSR	0x48
#define FTAHBC020_OFFSET_SLAVE19_BSR	0x4C
#define FTAHBC020_OFFSET_SLAVE21_BSR	0x54
#define FTAHBC020_OFFSET_SLAVE22_BSR	0x58
#define FTAHBC020_OFFSET_PCR		0x80
#define FTAHBC020_OFFSET_TCR		0x84
#define FTAHBC020_OFFSET_ICR		0x88

/*
 * AHB Slave n Base/Size Register
 */
#define FTAHBC020_BSR_BASE(x)	((x) & 0xfff00000)

#define FTAHBC020_BSR_SIZE_1M	(0x0 << 16)
#define FTAHBC020_BSR_SIZE_2M	(0x1 << 16)
#define FTAHBC020_BSR_SIZE_4M	(0x2 << 16)
#define FTAHBC020_BSR_SIZE_8M	(0x3 << 16)
#define FTAHBC020_BSR_SIZE_16M	(0x4 << 16)
#define FTAHBC020_BSR_SIZE_32M	(0x5 << 16)
#define FTAHBC020_BSR_SIZE_64M	(0x6 << 16)
#define FTAHBC020_BSR_SIZE_128M	(0x7 << 16)
#define FTAHBC020_BSR_SIZE_256M	(0x8 << 16)
#define FTAHBC020_BSR_SIZE_512M	(0x9 << 16)
#define FTAHBC020_BSR_SIZE_1G	(0xa << 16)
#define FTAHBC020_BSR_SIZE_2G	(0xb << 16)

/*
 * AHB Interrupt Control Register
 */
#define FTAHBC020_ICR_REMAP	(1 << 0)

#endif	/* __FTAHBC020_H */
