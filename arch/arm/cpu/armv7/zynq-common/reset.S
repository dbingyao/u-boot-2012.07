/*
 * (C) Copyright 2009 Faraday Technology
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
#include <common.h>

#include <asm/arch/xparameters_ps.h>

.global reset_cpu
reset_cpu:
	ldr	r0, =XPSS_SYS_CTRL_BASEADDR

	ldr	r1, =0xDF0D
	str	r1, [r0, #0x8]

	ldr	r1, =0x1
	str	r1, [r0, #0x200]

1:
	b	1b

