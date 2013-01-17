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

#include <config.h>
#include <version.h>

#include <asm/macro.h>
#include <faraday/ftahbc020.h>
#include <faraday/ftsdmc020.h>

/*
 * Memory Mapping
 */
#define SDRAM_DEFAULT_BASE	0x10000000
#ifdef CONFIG_SYS_RAM_REMAP
#define SDRAM_REMAPPED_BASE	0x00000000	/* remap location */
#define ROM_DEFAULT_BASE	0x00000000
#endif

#define GIC_DIST_BASE		0xe0001000

#define GIC_CPUIF_BASE		0xe0000100
#define GIC_CPUIF_CONTROL	0x0
#define GIC_CPUIF_PRIO_MASK	0x4
#define SYS_FLAGSS_OFFSET	0xfd0

/*
 * parameters for the SDRAM controller
 */
#define TP0_A		(CONFIG_FTSDMC020_BASE + FTSDMC020_OFFSET_TP0)
#define TP1_A		(CONFIG_FTSDMC020_BASE + FTSDMC020_OFFSET_TP1)
#define CR_A		(CONFIG_FTSDMC020_BASE + FTSDMC020_OFFSET_CR)
#define B0_BSR_A	(CONFIG_FTSDMC020_BASE + FTSDMC020_OFFSET_BANK0_BSR)
#define ACR_A		(CONFIG_FTSDMC020_BASE + FTSDMC020_OFFSET_ACR)

#define TP0_D		CONFIG_SYS_FTSDMC020_TP0
#define TP1_D		CONFIG_SYS_FTSDMC020_TP1
#define CR_D1		FTSDMC020_CR_IPREC
#define CR_D2		FTSDMC020_CR_ISMR
#define CR_D3		FTSDMC020_CR_IREF

#define B0_BSR_D1	(CONFIG_SYS_FTSDMC020_BANK0_BSR | \
			FTSDMC020_BANK_BASE(SDRAM_DEFAULT_BASE))
#ifdef CONFIG_SYS_RAM_REMAP
#define B0_BSR_D2	(CONFIG_SYS_FTSDMC020_BANK0_BSR | \
			FTSDMC020_BANK_BASE(SDRAM_REMAPPED_BASE))
#endif
#define ACR_D		FTSDMC020_ACR_TOC(0x18)

/*
 * numeric 7 segment display
 */
.macro	led, num
	write32	CONFIG_DEBUG_LED, \num
.endm

/*
 * Waiting for SDRAM to set up
 */
.macro	wait_sdram
	ldr	r0, =CONFIG_FTSDMC020_BASE
1:
	ldr	r1, [r0, #FTSDMC020_OFFSET_CR]
	cmp	r1, #0
	bne	1b
.endm

.globl lowlevel_init
lowlevel_init:
	mov	r11, lr

	ldr	r1, .LC6
	ldr	r2, .LC7
	mrc	p15, 0, r0, c0, c0, 0
	and	r0, r0, r1
	cmp	r0, r2
	bne	not_fmp626

	/* handle secondary processors */
	mrc	p15, 0, r0, c0, c0, 5
	ands	r0, r0, #0xf
	bne	secondary
not_fmp626:

#ifdef CONFIG_SYS_RAM_REMAP
	/* if REMAP bit is set -> memory had been initialzed */

	ldr	r0, =CONFIG_FTAHBC020_BASE
	ldr	r1, [r0, #FTAHBC020_OFFSET_ICR]
	tst	r1, #FTAHBC020_ICR_REMAP		@ test REMAP bit
	bne	skip_remap
#endif	/* CONFIG_SYS_RAM_REMAP */

	led	0x0

	bl	init_sdmc

	led	0x1

#ifdef CONFIG_SYS_RAM_REMAP
	/*
	 * copy U-Boot to RAM
	 */
copy_code:
	ldr	r0, =ROM_DEFAULT_BASE	/* r0 <- source address */
	ldr	r1, =SDRAM_DEFAULT_BASE	/* r1 <- target address */

	ldr	r2, .LC5
	ldr	r2, [r2]		/* r2 <- size of armboot */
	add	r2, r0, r2		/* r2 <- source end address */

	led	0x2

copy_loop:
	ldmia	r0!, {r3-r10}		/* copy from source address [r0] */
	stmia	r1!, {r3-r10}		/* copy to   target address [r1] */
	cmp	r0, r2			/* until source end addreee [r2] */
	ble	copy_loop

	led	0x3

	bl	remap

skip_remap:
	led	0x4
#endif	/* CONFIG_SYS_RAM_REMAP */

	/* everything is fine now */
	mov	lr, r11
	mov	pc, lr

#ifdef CONFIG_SYS_RAM_REMAP
.LC5:
	.word	_end_ofs
#endif	/* CONFIG_SYS_RAM_REMAP */
.LC6:
	.word	0xff0ffff8
.LC7:
	.word	0x66056268

/*
 * block secondary processors
 */
secondary:
	ldr	r0, =GIC_DIST_BASE
	ldr	r2, =GIC_CPUIF_BASE

2:	ldr	r1, [r0, #SYS_FLAGSS_OFFSET]
	cmp	r1, #0
	movne	pc, r1

	/* enable software interrupt */
	mov	r1, #0x1
	str	r1, [r2, #GIC_CPUIF_CONTROL]
	mov	r1, #0xf0
	str	r1, [r2, #GIC_CPUIF_PRIO_MASK]

	/* wait for interrupt */
	mov	r1, #0
	mcr	p15, 0, r1, c7, c0, 4

	/* disable software interrupt */
	mov	r1, #0x0
	str	r1, [r2, #GIC_CPUIF_CONTROL]
	mov	r1, #0x0
	str	r1, [r2, #GIC_CPUIF_PRIO_MASK]

	b	2b

/*
 * memory initialization
 */
init_sdmc:
	led	0x10

	/* set SDRAM register */

	write32	TP0_A, TP0_D
	led	0x11

	write32	TP1_A, TP1_D
	led	0x12

	/* set to precharge */
	write32	CR_A, CR_D1
	led	0x13

	wait_sdram
	led	0x14

	/* set mode register */
	write32	CR_A, CR_D2
	led	0x15

	wait_sdram
	led	0x16

	/* set to refresh */
	write32	CR_A, CR_D3
	led	0x17

	wait_sdram
	led	0x18

	write32	B0_BSR_A, B0_BSR_D1
	led	0x19

	write32	ACR_A, ACR_D
	led	0x1a

	mov	pc, lr

#ifdef CONFIG_SYS_RAM_REMAP
/*
 * This code will remap the memory ROM and SDRAM
 * ROM will be placed on 0x80000000 and SDRAM will jump to 0x0
 */
remap:
	ldr	r0, =CONFIG_FTSDMC020_BASE

	/* first adjust sdram */
	write32	B0_BSR_A, B0_BSR_D2

	/* then remap */
	ldr	r3, =CONFIG_FTAHBC020_BASE
	ldr	r4, [r3, #FTAHBC020_OFFSET_ICR]
	orr	r4, r4, #FTAHBC020_ICR_REMAP		@ Set REMAP bit
	str	r4, [r3, #FTAHBC020_OFFSET_ICR]

	mov	pc, lr
#endif	/* CONFIG_SYS_RAM_REMAP */
