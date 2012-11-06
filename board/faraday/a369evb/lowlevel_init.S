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

#include <config.h>
#include <asm/arch/a369.h>

	.macro	ddr2_setup

	adr	r3, ddr2_config_table
	ldr	r1, [r3], #4
1:
	ldr	r2, [r3], #4
	str	r2, [r1]
	ldr	r1, [r3], #4
	cmp	r1, #0
	bne	1b

	.endm

	.macro	ddr2_enable

	/* Clear SDRAM CKE, GPIO Hold, READ Hold */
	ldr	r1, =CONFIG_SCU_BASE
	mov	r2, #7
	str	r2, [r1, #0x14]

	/* Wait until sdram ready */
ddr2_wait:
	ldr	r2, [r0, #0x04]
	teq	r2, #0x100
	bne	ddr2_wait

	.endm

.globl lowlevel_init
lowlevel_init:
	ldr	r0, =CONFIG_FTDDRII030_BASE
	ldr	r1, [r0, #0x04]
	teq	r1, #0x100
	beq	_exit

	ddr2_setup
	ddr2_enable

	/* Change AHBC0 Slave 6(DDR2) Base Address and Size */
	mov	r1, #PHYS_SDRAM_2
	bic	r1, #0xffffff
	orr	r1, r1, #0x90000

	ldr	r0, =CONFIG_FTAHBC0_BASE
	/* Replace with AHBC0 Slave 24(Embedded RAM) */
	ldr	r2, =0x10000000
	str	r2, [r0, #0x60]
	str	r1, [r0, #0x18]

	/* GMAC and some others device access DDR through AHBC1 */
	ldr	r0, =CONFIG_FTAHBC1_BASE
	ldr	r2, =0x10100000
	str	r2, [r0, #0x34]
	str	r1, [r0, #0x24]

_exit:
	mov	pc, lr

ddr2_config_table:
	.long	0x93100000, 0x00005d02
	.long	0x93100004, 0x00000000
	.long	0x93100008, 0x00440a52
	.long	0x93100010, 0x10000045
	.long	0x93100018, 0x2b907274
	.long	0x9310001c, 0x00757251
	.long	0x93100020, 0x0c80124a
	.long	0x93100024, 0x00000000
	.long	0x93100028, 0x00000001
	.long	0x93100030, 0x141c3255
	.long	0x93100034, 0x5815f556
	.long	0x93100038, 0x00000000
	.long	0x93100040, 0x3f07061f
	.long	0x93100044, 0x06060606
	.long	0x93100048, 0x000000ff
	.long	0x93100004, 0x00000001
	.long	0

