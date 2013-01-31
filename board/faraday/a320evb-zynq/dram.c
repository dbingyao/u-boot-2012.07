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

#include <common.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	unsigned long sdram_base = PHYS_SDRAM_1;
	unsigned long expected_size = PHYS_SDRAM_1_SIZE;
	unsigned long actual_size;

	actual_size = get_ram_size((void *)sdram_base, expected_size);

	gd->ram_size = actual_size;

	printf("Detect %lu MiB SDRAM at 0x%08lx\n", actual_size >> 20, sdram_base);

	return 0;
}

void dram_init_banksize(void)
{
	unsigned long sdram_base;
	unsigned long actual_size;
	unsigned long expected_size;

	/* Memory at zynq board */
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size =  gd->ram_size;

	/* Memory at A320 */
	sdram_base = PHYS_SDRAM_2;
	expected_size = PHYS_SDRAM_2_SIZE;
	actual_size = get_ram_size((void *)sdram_base, expected_size);
	gd->bd->bi_dram[1].start = sdram_base;
	gd->bd->bi_dram[1].size  = actual_size;
	printf("Detect %lu MiB SDRAM at 0x%08lx\n", actual_size >> 20, sdram_base);

	/* detect SODIMM */
#if 0
	sdram_base = PHYS_SDRAM_3;
	actual_size = ftsdmc020_init_bank(1, sdram_base);
	gd->bd->bi_dram[2].start = sdram_base;
	gd->bd->bi_dram[2].size  = actual_size;
	printf("Detect %lu MiB SDRAM at 0x%08lx\n", actual_size >> 20, sdram_base);

	sdram_base = PHYS_SDRAM_4;
	actual_size = ftsdmc020_init_bank(2, sdram_base);
	gd->bd->bi_dram[3].start = sdram_base;
	gd->bd->bi_dram[3].size  = actual_size;
	printf("Detect %lu MiB SDRAM at 0x%08lx\n", actual_size >> 20, sdram_base);
#endif
}
