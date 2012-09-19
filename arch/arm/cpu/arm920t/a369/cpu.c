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
#include <asm/system.h>


static inline unsigned int get_line_size(void)
{
	unsigned int val;

	asm volatile (
		"mrc p15, 0, %0, c0, c0, 1"
		: "=r"(val) : : "cc");

	return  (1 << (3 +((val >> 12) & 0x3)));
}

int arch_cpu_init(void)
{
	return 0;
}

void flush_dcache_range(unsigned long start, unsigned long stop)
{
	unsigned long align, mask;

	align = get_line_size();
	mask  = ~(align - 1);

	/* aligned to cache line */
	stop  = (stop + (align - 1)) & mask;
	start = start & mask;

	asm volatile (
		"1:\n"
		"mcr p15,0,%0,c7,c14,1\n"	/* clean & invalidate d-cache line */
		"add %0,%0,%2\n"
		"cmp %0,%1\n"
		"blo 1b\n"
		"mov r3,#0\n"
		"mcr p15,0,r3,c7,c10,4\n"	/* drain write buffer */
		: "+r"(start)	/* output */
		: "r"(stop), "r"(align)	/* input */
		: "r3"	/* clobber list */
		);
}

void flush_dcache_all(void)
{
	asm volatile ("mov r0,#0\n"
		      "mcr p15,0,r0,c7,c14,0\n"       /* clean & invalidate d-cache all */
		      "mcr p15,0,r0,c7,c10,4\n"       /* drain write buffer */
		      : : : "r0");
}

void flush_cache(unsigned long start, unsigned long size)
{
	flush_dcache_range(start, start + size);
}

void invalidate_dcache_range(unsigned long start, unsigned long stop)
{
	unsigned long align, mask;

	align = get_line_size();
	mask  = ~(align - 1);

	/* aligned to cache line */
	stop  = (stop + (align - 1)) & mask;
	start = start & mask;

	asm volatile (
		"1:\n"
		"mcr p15,0,%0,c7,c6,1\n"	/* invalidate cache line */
		"add %0,%0,%2\n"
		"cmp %0,%1\n"
		"blo 1b\n"
		: "+r"(start)	/* output */
		: "r"(stop), "r"(align)	/* input */
		);
}

void invalidate_dcache_all(void)
{
	asm volatile (
		"mov r0,#0\n"
		"mcr p15,0,r0,c7,c6,0\n"	/* invalidate d-cache all */
		: : : "r0");
}


void invalidate_icache_all(void)
{
	asm volatile("mov r0, #0\n"
		     "mcr p15, 0, r0, c7, c5, 0\n"
		     : : : "r0");

}

void enable_caches(void)
{
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
	char cpu_name[32];
	uint32_t icache_sz, dcacahe_sz, line_sz;
	unsigned long cpu_id, ctr;

	asm volatile (
		"mrc p15, 0, %0, c0, c0, 0\n"
		"mrc p15, 0, %1, c0, c0, 1\n"
		: "=r"(cpu_id), "=r"(ctr) /* output */
		:	    /* input */
	);

	icache_sz   = 1 << (9 + ((ctr >>  6) & 0x07));
	dcacahe_sz  = 1 << (9 + ((ctr >> 18) & 0x07));
	line_sz = 1 << (3 + ((ctr >> 12) & 0x03));
	/* Implementor 0x66 is Faraday */
	if ((cpu_id >> 24) == 0x66) {
		switch (cpu_id >> 16) {
		case 0x6605:
			sprintf(cpu_name, "FA%xTE",  (unsigned int)(cpu_id >> 4)  & 0xFFF);
			break;
		}
	} else {
		sprintf(cpu_name, "ARM%x", (unsigned int)(cpu_id >> 4)  & 0xFFF);
	}

	/* print cpuinfo */
	printf("CPU:   %s %u MHz (I/D=%uKB/%uKB, LINE=%uBytes)\n",
		cpu_name, (unsigned int)(clk_get_rate("CPU") / 1000000),
		(unsigned int)(icache_sz >> 10),
		(unsigned int)(dcacahe_sz >> 10),
		(unsigned int)line_sz);

	printf("AHB:   %u MHz\n", (unsigned int)(clk_get_rate("AHB") / 1000000));
	printf("APB:   %u MHz\n", (unsigned int)(clk_get_rate("APB") / 1000000));

	return 0;
}
#endif	/* #ifdef CONFIG_DISPLAY_CPUINFO */
