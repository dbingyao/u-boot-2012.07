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

#include <common.h>
#include <nand.h>
#include <zynqpl.h>

#include <asm/arch/xparameters_ps.h>

DECLARE_GLOBAL_DATA_PTR;

extern void XIo_Out32(u32 OutAddress, u32 Value);

#ifdef CONFIG_FPGA
Xilinx_desc fpga = XILINX_XC7Z020_DESC(0);
#endif

int board_init(void)
{
	/* temporary hack to clear pending irqs before Linux as it 
	   will hang Linux */

	XIo_Out32(0xe0001014, 0x26d);

	/* temporary hack to take USB out of reset til the is fixed
	   in Linux */

	XIo_Out32(0xe000a204, 0x80);
	XIo_Out32(0xe000a208, 0x80);
	XIo_Out32(0xe000a040, 0x80);
	XIo_Out32(0xe000a040, 0x00);
	XIo_Out32(0xe000a040, 0x80);

	icache_enable();

#ifdef CONFIG_FPGA
	fpga_init();
	fpga_add(fpga_xilinx, &fpga);
#endif

	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

#if defined(PHYS_SDRAM_2) && defined (PHYS_SDRAM_2_SIZE)
void dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size =  gd->ram_size;

	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;
}
#endif

int dram_init(void)
{
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}

extern int ftgmac100_initialize(bd_t *bd);
int board_eth_init(bd_t *bd)
{
	return ftgmac100_initialize(bd);
}

extern int ftnandc021_probe(struct nand_chip *chip);
int board_nand_init(struct nand_chip *chip)
{
	uint32_t reg =  REG32(CONFIG_SCU_BASE + 0x204);

	/* page shift */
	switch((reg & 0x180) >> 7) {
	case 0:
		chip->page_shift = 9;	/* 512 */
		break;
	case 1:
		chip->page_shift = 11;	/* 2048 */
		break;
	case 2:
	case 3:
		chip->page_shift = 12;	/* 4096 */
		break;
	}

	/* block shift */
	switch((reg & 0x600) >> 9) {
	case 0: /* 16 pages */
		chip->phys_erase_shift = chip->page_shift + 4;
		break;
	case 1: /* 32 pages */
		chip->phys_erase_shift = chip->page_shift + 5;
		break;
	case 2: /* 64 pages */
		chip->phys_erase_shift = chip->page_shift + 6;
		break;
	case 3: /* 128 pages */
		chip->phys_erase_shift = chip->page_shift + 7;
		break;
	}

	/* address cycle */
	switch((reg & 0x60) >> 5) {
	case 0: /* NANDC_AP_3C: */
		chip->priv = (void *)3;
		break;
	case 1: /* NANDC_AP_4C: */
		chip->priv = (void *)4;
		break;
	case 2: /* NANDC_AP_5C: */
	case 3:
		chip->priv = (void *)5;
		break;
	}

	return ftnandc021_probe(chip);
}

extern int ftsdc010_mmc_init(int dev_index);
int board_mmc_init(bd_t *bis)
{
	/* Clock Setup: SD = 133MHz, SSP = APB (SPI mode) */
	REG32(CONFIG_SCU_BASE + 0x22C) = 0x000A0A0A;

	ftsdc010_mmc_init(0);
	return 0;
}
