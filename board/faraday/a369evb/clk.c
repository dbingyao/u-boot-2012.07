/*
 * (C) Copyright 2010
 * Faraday Technology Inc. <www.faraday-tech.com>
 * Dante Su <dantesu@gmail.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>

static ulong clk_get_rate_ahb(void)
{
	ulong mul = 4, clk;

	/* PLL1 Enabled */
	if (REG32(CONFIG_SCU_BASE + 0x20) & (1 << 0)) {
		int retry = 0x1FFFF;
		/* wait until PLL1 become stable */
		while(--retry > 0 && !(REG32(CONFIG_SCU_BASE + 0x20) & (1 << 1)));
		mul = (REG32(CONFIG_SCU_BASE + 0x20) >> 24) & 0x3F;
	}

	clk = (CONFIG_MAIN_CLK * mul) >> 3;
	return clk;
}

static ulong clk_get_rate_apb(void)
{
	return clk_get_rate_ahb() >> 1;
}

static ulong clk_get_rate_cpu(void)
{
	ulong clk = clk_get_rate_ahb();

#ifndef CONFIG_A369_FA606TE_PLATFORM
	if (!(REG32(CONFIG_SCU_BASE + 0x204) & (1 << 2))) {

		switch ((REG32(CONFIG_SCU_BASE + 0x08) >> 3) & 0x03) {
		case 0:
			clk = clk << 0;
			break;
		case 1:
			clk = clk << 1;
			break;
		default:
			clk = clk << 2;
			break;
		}

	}
#endif

	return clk;
}

ulong clk_get_rate(char *id)
{
	ulong clk = 0;

	if (!strcmp(id, "AHB"))
		clk = clk_get_rate_ahb();
	else if (!strcmp(id, "APB"))
		clk = clk_get_rate_apb();
	else if (!strcmp(id, "CPU"))
		clk = clk_get_rate_cpu();
	else if (!strcmp(id, "SDC"))
		clk = clk_get_rate_ahb();
	else if (!strcmp(id, "I2C"))
		clk = clk_get_rate_apb();
	else if (!strcmp(id, "SPI") || !strcmp(id, "SSP"))
		clk = clk_get_rate_apb();

	return clk;
}
