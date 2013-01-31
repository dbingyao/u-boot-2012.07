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

/*
 * CPU specific code
 */

#include <common.h>
#include <command.h>
#include <asm/armv7.h>
#include <asm/system.h>


#ifdef CONFIG_ARCH_EARLY_INIT_R
unsigned int io_table[] = { CONFIG_FTUART010_BASE,
			    CONFIG_FTPWMTMR010_BASE,
			    CONFIG_SCU_BASE,
			    CONFIG_FTWDT010_BASE,
			    CONFIG_FTGMAC100_BASE,
			    CONFIG_FTNANDC021_BASE,
			    CONFIG_FTSDC010_BASE };

#define NUM_IOS (sizeof(io_table)/sizeof(unsigned int))
#endif

int arch_cpu_init(void)
{
	return 0;
}


void enable_caches(void)
{
/* If skip low level init and D/I CACHE is off,
 * We disable them here. Otherwise, it already
 * done at start.S
 */
#if !defined(CONFIG_SYS_DCACHE_OFF)
	dcache_enable();
#else
	dcache_disable();
#endif

#if !defined(CONFIG_SYS_ICACHE_OFF)
	icache_enable();
#else
	icache_disable();
#endif
	printf ("Data (writethrough) Cache is %s\n",
		dcache_status() ? "ON" : "OFF");

	printf ("Instruction Cache is %s\n",
		icache_status() ? "ON" : "OFF");

}

#ifdef CONFIG_DISPLAY_CPUINFO
int print_cpuinfo(void)
{
	char cpu_name[100];
	u32 cpu_id, cpu_reg;
	u32 imp_code, line_len, level, cache_type, level_start_bit;


	asm volatile (
		"mrc p15, 0, %0, c0, c0, 0\n"
		: "=r"(cpu_id)/* output */
		:	    /* input */
	);

	imp_code = cpu_id >> 24;
	if (imp_code == 0x66) { /* Faraday */
		switch (cpu_id >> 16) {
		case 0x6605:
			sprintf(cpu_name, "FA%xTE",  (unsigned int)(cpu_id >> 4)  & 0xFFF);
			break;
		}
	} else {
		sprintf(cpu_name, "Main ID register (0x%08x)", cpu_id);
	}

	/* Read current CP15 Cache Size ID Register */
	asm volatile ("mrc p15, 1, %0, c0, c0, 0" : "=r" (cpu_reg));
	line_len = ((cpu_reg & CCSIDR_LINE_SIZE_MASK) >>
			CCSIDR_LINE_SIZE_OFFSET) + 2;
	/* Converting from words to bytes */
	line_len += 2;
	/* converting from log2(linelen) to linelen */
	line_len = 1 << line_len;

	/* print cpuinfo */
	printf("CPU:   %s %u MHz (Cache size= %u Bytes)\n",
		cpu_name, (unsigned int)(clk_get_rate("CPU") / 1000000),
		(unsigned int)line_len);

	level_start_bit = 0;
	asm volatile ("mrc p15,1,%0,c0,c0,1" : "=r" (cpu_reg));
	for (level = 0; level < 7; level++) {
		cache_type = (cpu_reg >> level_start_bit) & 0x7;
		if (cache_type == ARMV7_CLIDR_CTYPE_DATA_ONLY)
			printf("Cache Level %d type: Data only\n", level);
		else if (cache_type == ARMV7_CLIDR_CTYPE_INSTRUCTION_ONLY)
			printf("Cache Level %d type: Instruction only\n", level);
		else if (cache_type == ARMV7_CLIDR_CTYPE_INSTRUCTION_DATA)
			printf("Cache Level %d type: Instruction and Data\n", level);
		else if (cache_type == ARMV7_CLIDR_CTYPE_UNIFIED)
			printf("Cache Level %d type: Unified\n", level);

		level_start_bit += 3;
	}

	printf("AHB:   %u MHz\n", (unsigned int)(clk_get_rate("AHB") / 1000000));
	printf("APB:   %u MHz\n", (unsigned int)(clk_get_rate("APB") / 1000000));

	return 0;
}
#endif	/* #ifdef CONFIG_DISPLAY_CPUINFO */

#ifdef CONFIG_ARCH_EARLY_INIT_R
#if !defined(CONFIG_SYS_DCACHE_OFF)
int arch_early_init_r(void)
{
	DECLARE_GLOBAL_DATA_PTR;
	int i;
	u32 *page_table = (u32 *)gd->tlb_addr;
	u32 ca;

	dcache_disable();
	asm volatile ("mov r3,#0\n"
		      "mcr p15, 0, r3, c8, c6, 0\n"  /* invalidate DTLB all */
		      : : : "r3");
	printf("Uncaching controller register memory region ...\n");
	for ( i=0; i < NUM_IOS ; i ++) {

		ca = (io_table[i] >> 20);
		page_table[ca] &= ~0xC;
	}
	dcache_enable();
}
#endif
#endif
