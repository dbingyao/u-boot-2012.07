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
#include <asm/arch/a369.h>

#define FTWDT010_OFFSET_WDLOAD		0x4
#define FTWDT010_OFFSET_WDRESTART	0x8
#define FTWDT010_OFFSET_WDCONTROL	0xC

.global reset_cpu
reset_cpu:
	ldr	r0, =CONFIG_FTWDT010_BASE

	ldr	r1, =0
	str	r1, [r0, #FTWDT010_OFFSET_WDCONTROL]

	ldr	r1, =0x1000
	str	r1, [r0, #FTWDT010_OFFSET_WDLOAD]

	ldr	r1, =0x3
	str	r1, [r0, #FTWDT010_OFFSET_WDCONTROL]

	ldr	r1, =0x5AB9
	str	r1, [r0, #FTWDT010_OFFSET_WDRESTART]

	mov	pc, lr

