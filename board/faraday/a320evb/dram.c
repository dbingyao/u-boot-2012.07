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

	if (expected_size != actual_size)
		printf("Warning: Only %lu of %lu MiB SDRAM is working\n",
				actual_size >> 20, expected_size >> 20);

	return 0;
}

void dram_init_banksize(void)
{
	unsigned long sdram_base;
	unsigned long actual_size;

	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size =  gd->ram_size;

	/* detect SODIMM */

	sdram_base = PHYS_SDRAM_2;
	actual_size = ftsdmc020_init_bank(1, sdram_base);
	gd->bd->bi_dram[1].start = sdram_base;
	gd->bd->bi_dram[1].size  = actual_size;

	sdram_base = PHYS_SDRAM_3;
	actual_size = ftsdmc020_init_bank(2, sdram_base);
	gd->bd->bi_dram[2].start = sdram_base;
	gd->bd->bi_dram[2].size  = actual_size;
}
