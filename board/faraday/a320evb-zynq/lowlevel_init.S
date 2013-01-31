/*
 * (C) Copyright 2013 Faraday Technology
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
#include <version.h>

#include <asm/macro.h>
#include <faraday/ftahbc020.h>
#include <faraday/ftsdmc020.h>
#include <asm/arch/xparameters_ps.h>

#define FTPMU010_PA_BASE 0x98100000

/*
 * Memory Mapping
 */
#define SDRAM_BANK0_BASE	PHYS_SDRAM_2
#define SDRAM_BANK1_BASE	PHYS_SDRAM_3
#define SDRAM_BANK2_BASE	PHYS_SDRAM_4

/*
 * parameters for the SDRAM controller
 */
#define TP0_A		(CONFIG_FTSDMC020_BASE + FTSDMC020_OFFSET_TP0)
#define TP1_A		(CONFIG_FTSDMC020_BASE + FTSDMC020_OFFSET_TP1)
#define CR_A		(CONFIG_FTSDMC020_BASE + FTSDMC020_OFFSET_CR)
#define B0_BSR_A	(CONFIG_FTSDMC020_BASE + FTSDMC020_OFFSET_BANK0_BSR)
#define B1_BSR_A	(CONFIG_FTSDMC020_BASE + FTSDMC020_OFFSET_BANK1_BSR)
#define B2_BSR_A	(CONFIG_FTSDMC020_BASE + FTSDMC020_OFFSET_BANK2_BSR)
#define ACR_A		(CONFIG_FTSDMC020_BASE + FTSDMC020_OFFSET_ACR)

#define TP0_D		CONFIG_SYS_FTSDMC020_TP0
#define TP1_D		CONFIG_SYS_FTSDMC020_TP1
#define CR_D1		FTSDMC020_CR_IPREC
#define CR_D2		FTSDMC020_CR_ISMR
#define CR_D3		FTSDMC020_CR_IREF

#define B0_BSR_D1	(CONFIG_SYS_FTSDMC020_BANK0_BSR | \
			FTSDMC020_BANK_BASE(SDRAM_BANK0_BASE))

#define B1_BSR_D1	(CONFIG_SYS_FTSDMC020_BANK1_BSR | \
			FTSDMC020_BANK_BASE(SDRAM_BANK1_BASE))

#define B2_BSR_D1	(CONFIG_SYS_FTSDMC020_BANK2_BSR | \
			FTSDMC020_BANK_BASE(SDRAM_BANK2_BASE))

#define ACR_D		FTSDMC020_ACR_TOC(0x18)

#define XPSS_SYS_CTRL_CPU_RST 0x244

	.macro	zynq_setup

	# unlock SLCR
	ldr	r1, =(XPSS_SYS_CTRL_BASEADDR + 8)
	ldr	r2, =0xDF0D
	str	r2, [r1]

	# remap DDR to zero
	# FILTERSTART
	ldr	r1, =(XPSS_SCU_BASEADDR + 0x40)
	ldr	r2, =0
	str	r2, [r1]

	# Device config APB
	# unlock the PCAP
	ldr	r1, =(XPSS_DEV_CFG_APB_BASEADDR + 0x34)
	ldr	r2, =0x757BDF0D
	str	r2, [r1]
	ldr	r1, =(XPSS_DEV_CFG_APB_BASEADDR + 0x28)
	ldr	r2, =0xFFFFFFFF
	str	r2, [r1]

	# OCM_CFG
	# Mask out the ROM
	# map ram into upper addresses
	ldr	r1, =(XPSS_SYS_CTRL_BASEADDR +0x910)
	ldr	r2, =0x1F
	str	r2, [r1]
	
	# FPGA_RST_CTRL
	# clear resets on AXI fabric ports
	ldr	r1, =(XPSS_SYS_CTRL_BASEADDR + 0x240)
	ldr	r2, =0x0
	str	r2, [r1]

	# TZ_DDR_RAM
	# Set DDR trust zone non-secure
	ldr	r1, =(XPSS_SYS_CTRL_BASEADDR + 0x430)
	ldr	r2, =0xFFFFFFFF
	str	r2, [r1]

	# set urgent bits with register
	ldr	r1, =(XPSS_SYS_CTRL_BASEADDR + 0x61C)
	ldr	r2, =0
	str	r2, [r1]

	# urgent write, ports S2/S3
	ldr	r1, =(XPSS_SYS_CTRL_BASEADDR + 0x600)
	ldr	r2, =0xC
	str	r2, [r1]

	# CPU1 is putted into loop at Stage 0(BootRom)
	# Wake up CPU1 here and put it into another loop
	# It will be released during linux OS initialization

	#prepare the place where CPU1 go when wake up
	adr	r0, secondary
	mov	r1, #0x0
	adr	r2, base_addr
	add	r2, r2, #0xc
copy_loop:
	ldmia	r0!, {r9-r10}
	stmia	r1!, {r9-r10}
	cmp	r0, r2
	blo	copy_loop

	# Wake up CPU1
	ldr	r1, =(XPSS_SYS_CTRL_BASEADDR + XPSS_SYS_CTRL_CPU_RST)
	mov	r2, #0x22 	/* Stop CLK and reset CPU1 */
	str	r2, [r1]
	mov	r2, #0x20 	/* Enable CPU1 */
	str	r2, [r1]
	mov	r2, #0x0	/* Enable CLK for CPU1 */
	str	r2, [r1]

	# relock SLCR
	ldr	r1, =(XPSS_SYS_CTRL_BASEADDR + 0x4)
	ldr	r2, =0x767B
	str	r2, [r1]

	.endm
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
	mov 	r11, lr
	zynq_setup

	/* if REMAP bit is set -> memory had been initialzed */

	ldr	r0, =CONFIG_FTAHBC020_BASE
	ldr	r1, [r0, #FTAHBC020_OFFSET_ICR]
	tst	r1, #FTAHBC020_ICR_REMAP		@ test REMAP bit
	bne	skip_remap

	led	0x0

	bl	init_sdmc

	led	0x1

	bl	remap

skip_remap:
	led	0x2

	/* everything is fine now */
	mov	lr, r11
	mov	pc, lr

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

/*
 * This code will remap the memory ROM and SDRAM
 * SDRAM will jump to 0xA0000000
 */
remap:
	/* then remap */
	ldr	r3, =CONFIG_FTAHBC020_BASE
	ldr	r4, [r3, #FTAHBC020_OFFSET_ICR]
	orr	r4, r4, #FTAHBC020_ICR_REMAP		@ Set REMAP bit
	str	r4, [r3, #FTAHBC020_OFFSET_ICR]

	/* AHB Slave 21 External device 1 */
	ldr	r4, =0x000A0000
	str	r4, [r3, #0x54]

	/* AHB Slave 6 SRAM/ROM/Flash */
	ldr	r4, =0xA0070000
	str	r4, [r3, #0x18]

	mov	pc, lr

/*
 * This code will be moved to address 0 for CPU1 wake up
 */
secondary:
	adr 	r1, base_addr
	ldmia 	r1, {r2, r3}

	/*
	 * Invalidate L1 I/D
	 */
	mov	r0, #0			@ set up for MCR
	mcr	p15, 0, r0, c8, c7, 0	@ invalidate TLBs
	mcr	p15, 0, r0, c7, c5, 0	@ invalidate icache
	mcr	p15, 0, r0, c7, c5, 6	@ invalidate BP array
	dsb
	isb

	/*
	 * disable MMU stuff and caches
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002000	@ clear bits 13 (--V-)
	bic	r0, r0, #0x00000007	@ clear bits 2:0 (-CAM)
	orr	r0, r0, #0x00000002	@ set bit 1 (--A-) Align
	orr	r0, r0, #0x00000800	@ set bit 11 (Z---) BTB
	orr	r0, r0, #0x00001000	@ set bit 12 (I) I-cache
	mcr	p15, 0, r0, c1, c0, 0
	isb	sy

_loop:
	ldr 	r1, [r3, #0x50]
	cmp	r1, #0
	movne	pc, r1

	/* enable software interrupt */
	mov	r1, #0x1
	str	r1, [r2, #GIC_CPU_CTRL]
	mov	r1, #0xf0
	str	r1, [r2, #GIC_CPU_PRIMASK]

	/* ensure the following code reside in the same I-cache line */
	.align 5

	/* wait for interrupt */
	wfi

	/* disable software interrupt */
	mov	r1, #0x0
	str	r1, [r2, #GIC_CPU_CTRL]
	mov	r1, #0x0
	str	r1, [r2, #GIC_CPU_PRIMASK]

	b	_loop

base_addr:
	.long	GIC_CPU_BASE
	.long	FTPMU010_PA_BASE
	.long	0xFFFFFFFF
