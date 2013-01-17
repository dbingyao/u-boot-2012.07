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
#include <common.h>
#include <asm/io.h>
#include <faraday/ftsdmc020.h>

static int ftsdmc020_test_address(unsigned int addr, unsigned bit)
{
	*(volatile unsigned int *)addr = 0x12345678;
	*(volatile unsigned int *)(addr + (1 << bit)) = 0x5555aaaa;

	if ((*(volatile unsigned int *)addr) == 0x5555aaaa)	{	/* wrapped */
		return 0;

	} else if (*(volatile unsigned int *)(addr + (1 << bit)) != 0x5555aaaa) {
		return 0;

	} else {
		return 1;
	}
}

static void ftsdmc020_setup_bank(int bank, unsigned int bsr)
{
	int *addr = (int *)(CONFIG_FTSDMC020_BASE  + FTSDMC020_OFFSET_BANK0_BSR + (bank * 4));

	writel (bsr, addr);
}

unsigned int ftsdmc020_init_bank(int bank, unsigned int base)
{
	unsigned int size = 0;
	unsigned int bsr;

	bsr	= FTSDMC020_BANK_ENABLE
		| FTSDMC020_BANK_BASE(base)
		| FTSDMC020_BANK_DDW_X4
		| FTSDMC020_BANK_DSZ_256M
		| FTSDMC020_BANK_MBW_32
		| FTSDMC020_BANK_SIZE_256M;

	ftsdmc020_setup_bank(bank, bsr);

	bsr	= 0;

	if (ftsdmc020_test_address(base, 12)) {
		if (ftsdmc020_test_address(base, 27)) {
			printf("bank %d:  x4 256Mbit * 8 = 256MByte @ 0x%08x\n", bank, base);
			bsr	= FTSDMC020_BANK_ENABLE
				| FTSDMC020_BANK_BASE(base)
				| FTSDMC020_BANK_DDW_X4
				| FTSDMC020_BANK_DSZ_256M
				| FTSDMC020_BANK_MBW_32
				| FTSDMC020_BANK_SIZE_256M;
			size = 256 * 1024 * 1024;

		} else if (ftsdmc020_test_address(base, 26)) {
			printf("bank %d:  x4 128Mbit * 8 = 128MByte @ 0x%08x\n", bank, base);
			bsr	= FTSDMC020_BANK_ENABLE
				| FTSDMC020_BANK_BASE(base)
				| FTSDMC020_BANK_DDW_X4
				| FTSDMC020_BANK_DSZ_128M
				| FTSDMC020_BANK_MBW_32
				| FTSDMC020_BANK_SIZE_128M;
			size = 128 * 1024 * 1024;

		} else if (ftsdmc020_test_address(base, 23)) {
			printf("bank %d:  x4  16Mbit * 8 =  16MByte @ 0x%08x\n", bank, base);
			bsr	= FTSDMC020_BANK_ENABLE
				| FTSDMC020_BANK_BASE(base)
				| FTSDMC020_BANK_DDW_X4
				| FTSDMC020_BANK_DSZ_16M
				| FTSDMC020_BANK_MBW_32
				| FTSDMC020_BANK_SIZE_16M;
			size = 16 * 1024 * 1024;
		}

	} else if (ftsdmc020_test_address(base, 11)) {
		if (ftsdmc020_test_address(base, 26)) {
			printf("bank %d:  x8 256Mbit * 4 = 128MByte @ 0x%08x\n", bank, base);
			bsr	= FTSDMC020_BANK_ENABLE
				| FTSDMC020_BANK_BASE(base)
				| FTSDMC020_BANK_DDW_X8
				| FTSDMC020_BANK_DSZ_256M
				| FTSDMC020_BANK_MBW_32
				| FTSDMC020_BANK_SIZE_128M;
			size = 128 * 1024 * 1024;

		} else if (ftsdmc020_test_address(base, 25)) {
			printf("bank %d:  x4  64Mbit * 8 =  64MByte @ 0x%08x\n", bank, base);
			printf( "    or:  x8 128Mbit * 4 =  64MByte\n");
			bsr	= FTSDMC020_BANK_ENABLE
				| FTSDMC020_BANK_BASE(base)
				| FTSDMC020_BANK_DDW_X4
				| FTSDMC020_BANK_DSZ_64M
				| FTSDMC020_BANK_MBW_32
				| FTSDMC020_BANK_SIZE_64M;
			size = 64 * 1024 * 1024;
		}

	} else if (ftsdmc020_test_address(base, 10)) {
		if (ftsdmc020_test_address(base, 25)) {
			printf("bank %d:  x16 256Mbit * 2 = 64MByte @ 0x%08x\n", bank, base);
			bsr	= FTSDMC020_BANK_ENABLE
				| FTSDMC020_BANK_BASE(base)
				| FTSDMC020_BANK_DDW_X16
				| FTSDMC020_BANK_DSZ_256M
				| FTSDMC020_BANK_MBW_32
				| FTSDMC020_BANK_SIZE_64M;
			size = 64 * 1024 * 1024;

		} else if (ftsdmc020_test_address(base, 24)) {
			printf("bank %d:  x8  64Mbit * 4 =  32MByte @ 0x%08x\n", bank, base);
			printf( "    or: x16 128Mbit * 2 =  32MByte\n");
			bsr	= FTSDMC020_BANK_ENABLE
				| FTSDMC020_BANK_BASE(base)
				| FTSDMC020_BANK_DDW_X8
				| FTSDMC020_BANK_DSZ_64M
				| FTSDMC020_BANK_MBW_32
				| FTSDMC020_BANK_SIZE_32M;
			size = 32 * 1024 * 1024;

		} else if (ftsdmc020_test_address(base, 22)) {
			printf("bank %d:  x8  16Mbit * 4 =   8MByte @ 0x%08x\n", bank, base);
			bsr	= FTSDMC020_BANK_ENABLE
				| FTSDMC020_BANK_BASE(base)
				| FTSDMC020_BANK_DDW_X8
				| FTSDMC020_BANK_DSZ_16M
				| FTSDMC020_BANK_MBW_32
				| FTSDMC020_BANK_SIZE_8M;
			size = 8 * 1024 * 1024;
		}

	} else if (ftsdmc020_test_address(base, 9)) {
		if (ftsdmc020_test_address(base, 24)) {
			printf("bank %d: x32 256Mbit * 1 =  32MByte @ 0x%08x\n", bank, base);
			bsr	= FTSDMC020_BANK_ENABLE
				| FTSDMC020_BANK_BASE(base)
				| FTSDMC020_BANK_DDW_X32
				| FTSDMC020_BANK_DSZ_256M
				| FTSDMC020_BANK_MBW_32
				| FTSDMC020_BANK_SIZE_32M;
			size = 32 * 1024 * 1024;

		} else if (ftsdmc020_test_address(base, 23)) {
			printf("bank %d: x32 128Mbit * 1 =  16MByte @ 0x%08x\n", bank, base);
			printf( "    or: x16  64Mbit * 2 =  16MByte\n");
			bsr	= FTSDMC020_BANK_ENABLE
				| FTSDMC020_BANK_BASE(base)
				| FTSDMC020_BANK_DDW_X32
				| FTSDMC020_BANK_DSZ_128M
				| FTSDMC020_BANK_MBW_32
				| FTSDMC020_BANK_SIZE_16M;
			size = 16 * 1024 * 1024;

		} else if (ftsdmc020_test_address(base, 22)) {
			printf("bank %d: x32  64Mbit * 1 =   8MByte @ 0x%08x\n", bank, base);
			bsr	= FTSDMC020_BANK_ENABLE
				| FTSDMC020_BANK_BASE(base)
				| FTSDMC020_BANK_DDW_X32
				| FTSDMC020_BANK_DSZ_64M
				| FTSDMC020_BANK_MBW_32
				| FTSDMC020_BANK_SIZE_8M;
			size = 8 * 1024 * 1024;

		} else if (ftsdmc020_test_address(base, 21)) {
			printf("bank %d: x16  16Mbit * 2 =   4MByte @ 0x%08x\n", bank, base);
			bsr	= FTSDMC020_BANK_ENABLE
				| FTSDMC020_BANK_BASE(base)
				| FTSDMC020_BANK_DDW_X16
				| FTSDMC020_BANK_DSZ_16M
				| FTSDMC020_BANK_MBW_32
				| FTSDMC020_BANK_SIZE_4M;
			size = 4 * 1024 * 1024;
		}
	}

	ftsdmc020_setup_bank(bank, bsr);
	return size;
}


