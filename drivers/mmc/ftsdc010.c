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
#include <malloc.h>
#include <part.h>
#include <mmc.h>

#include <asm/io.h>
#include <asm/errno.h>
#include <asm/byteorder.h>

#include "ftsdc010.h"

#define SD_REG32(chip, off)		*(volatile uint32_t *)((uint8_t *)(chip)->iobase + (off))

typedef struct {
	void*    iobase;

	uint32_t wprot;	/* write protected (locked) */
	uint32_t rate;	/* actual SD clock in Hz */
	uint32_t sclk;	/* FTSDC010 source clock in Hz */
	uint32_t fifo;	/* fifo depth in bytes */
	uint32_t acmd;

} __attribute__ ((aligned (4))) ftsdc010_chip_t;

static inline int
ftsdc010_send_cmd(struct mmc *mmc, struct mmc_cmd *mmc_cmd)
{
	ftsdc010_chip_t* chip = mmc->priv;
	uint32_t timeout;

	uint32_t cmd   = mmc_cmd->cmdidx;
	uint32_t arg   = mmc_cmd->cmdarg;
	uint32_t flags = mmc_cmd->resp_type;

	cmd |= CMD_EN;

	if (chip->acmd) {
		cmd |= CMD_APP;
		chip->acmd = 0;
	}

	if (flags & MMC_RSP_PRESENT)
		cmd |= CMD_WAIT_RSP;

	if (flags & MMC_RSP_136)
		cmd |= CMD_LONG_RSP;

	SD_REG32(chip, REG_SCR) = STR_RSP_ERR | STR_RSP | STR_CMD;

	SD_REG32(chip, REG_ARG) = arg;

	SD_REG32(chip, REG_CMD) = cmd;

	if ((flags & (MMC_RSP_PRESENT | MMC_RSP_136)) == 0) {
		for (timeout = 250000; timeout > 0; --timeout) {
			if (SD_REG32(chip, REG_STR) & STR_CMD) {
				SD_REG32(chip, REG_SCR) = STR_CMD;
				break;
			}
			udelay(1);
		}
	} else {
		for (timeout = 250000; timeout > 0; --timeout) {
			uint32_t st = SD_REG32(chip, REG_STR);
			if (st & STR_RSP) {
				SD_REG32(chip, REG_SCR) = STR_RSP;
				if (flags & MMC_RSP_136) {
					mmc_cmd->response[0] = SD_REG32(chip, REG_RSP3);
					mmc_cmd->response[1] = SD_REG32(chip, REG_RSP2);
					mmc_cmd->response[2] = SD_REG32(chip, REG_RSP1);
					mmc_cmd->response[3] = SD_REG32(chip, REG_RSP0);
				} else {
					mmc_cmd->response[0] = SD_REG32(chip, REG_RSP0);
				}
				break;
			} else if (st & STR_RSP_ERR) {
				SD_REG32(chip, REG_SCR) = STR_RSP_ERR;
				printf("ftsdc010: rsp err (cmd=%d, st=0x%x)\n", mmc_cmd->cmdidx, st);
				return TIMEOUT;
			}
			udelay(1);
		}
	}

	if (timeout == 0) {
		printf("ftsdc010: cmd timeout (op code=%d)\n", mmc_cmd->cmdidx);
		return TIMEOUT;
	}

	if (mmc_cmd->cmdidx == MMC_CMD_APP_CMD)
		chip->acmd = 1;

	return 0;
}

static int
ftsdc010_wait(struct mmc *mmc)
{
	ftsdc010_chip_t* chip = mmc->priv;
	uint32_t mask = STR_DAT | STR_DAT_END | STR_DAT_ERR;
	uint32_t timeout;

	for (timeout = 250000; timeout; --timeout) {
		uint32_t st = SD_REG32(chip, REG_STR);
		SD_REG32(chip, REG_SCR) = (st & mask);

		if (st & STR_DAT_ERR) {
			printf("ftsdc010: data error!(st=0x%x)\n", st);
			return TIMEOUT;
		} else if (st & STR_DAT_END) {
			break;
		}
		udelay(1);
	}

	if (timeout == 0) {
		printf("ftsdc010: wait timeout\n");
		return TIMEOUT;
	}

	return 0;
}

static void
ftsdc010_clkset(ftsdc010_chip_t* chip, uint32_t rate)
{
	uint32_t div;
	uint32_t clk = chip->sclk;

	for (div = 0; div < 0x7F; ++div) {
		if (rate >= clk / (2 * (div + 1)))
			break;
	}
	SD_REG32(chip, REG_CLK) = CLK_SD | div;

	chip->rate = clk / (2 * (div + 1));

#if 0
	if (chip->rate >= 1000000)
		printf("ftsdc010: clock=%d MHz (div=%d, ahb=%d)\n", chip->rate / 1000000, div, clk);
	else
		printf("ftsdc010: clock=%d KHz (div=%d, ahb=%d)\n", chip->rate / 1000, div, clk);
#endif
}

static inline int
ftsdc010_is_ro(struct mmc *mmc)
{
	ftsdc010_chip_t* chip = mmc->priv;
	const uint8_t *csd = (const uint8_t *)mmc->csd;

	if (chip->wprot || (csd[1] & 0x30))
		return 1;

	return 0;
}

/*
 * u-boot mmc api
 */
static int ftsdc010_request(struct mmc *mmc,
                            struct mmc_cmd *cmd,
                            struct mmc_data *data)
{
	int rc;
	uint32_t len = 0;
	ftsdc010_chip_t* chip = mmc->priv;

	if (data && (data->flags & MMC_DATA_WRITE) && chip->wprot) {
		printf("ftsdc010: the card is write protected!\n");
		return UNUSABLE_ERR;
	}

	if (data) {
		uint32_t dcr;

		len = data->blocksize * data->blocks;

		/* 1. data disable + fifo reset */
		SD_REG32(chip, REG_DCR) = DCR_FIFO_RESET;

		/* 2. clear status register */
		SD_REG32(chip, REG_SCR) = STR_DAT_END | STR_DAT | STR_DAT_ERR | STR_TXRDY | STR_RXRDY;

		/* 3. data timeout (1 sec) */
		SD_REG32(chip, REG_DTR) = chip->rate;

		/* 4. data length (bytes) */
		SD_REG32(chip, REG_DLR) = len;

		/* 5. data enable */
		dcr = (ffs(data->blocksize) - 1) | DCR_EN;
		if (data->flags & MMC_DATA_WRITE)
			dcr |= DCR_WR;
		SD_REG32(chip, REG_DCR) = dcr;
	}

	rc = ftsdc010_send_cmd(mmc, cmd);
	if (rc) {
		printf("ftsdc010: sending CMD%d failed\n", cmd->cmdidx);
		return rc;
	}

	if (!data)
		return rc;

	if (data->flags & MMC_DATA_WRITE) {
		const uint8_t* buf = (const uint8_t*)data->src;

		while (len > 0) {
			int wlen;

			/* wait data ready */
			while(!(SD_REG32(chip, REG_STR) & STR_TXRDY));
			SD_REG32(chip, REG_SCR) = STR_TXRDY;

			/* write bytes to ftsdc010 */
			for (wlen = 0; wlen < len && wlen < chip->fifo; ) {
				SD_REG32(chip, REG_DWR) = *(uint32_t *)buf;
				buf  += 4;
				wlen += 4;
			}

			len -= wlen;
		}

	} else {
		uint8_t *buf = (uint8_t *)data->dest;

		while (len > 0) {
			int rlen;

			/* wait data ready */
			while(!(SD_REG32(chip, REG_STR) & STR_RXRDY));
			SD_REG32(chip, REG_SCR) = STR_RXRDY;

			/* fetch bytes from ftsdc010 */
			for (rlen = 0; rlen < len && rlen < chip->fifo; ) {
				*(uint32_t *)buf = SD_REG32(chip, REG_DWR);
				buf  += 4;
				rlen += 4;
			}

			len -= rlen;
		}
	}

	rc = ftsdc010_wait(mmc);

	return rc;
}

static void ftsdc010_set_ios(struct mmc *mmc)
{
	ftsdc010_chip_t* chip = mmc->priv;

	ftsdc010_clkset(chip, mmc->clock);

	if (mmc->clock > 25000000)
		SD_REG32(chip, REG_CLK) |= CLK_HISPD;
	else
		SD_REG32(chip, REG_CLK) &= ~CLK_HISPD;

	SD_REG32(chip, REG_BUS) &= 0xFFFFFFF8;
	switch(mmc->bus_width) {
	case 4:
		SD_REG32(chip, REG_BUS) |= 0x04;
		break;
	case 8:
		SD_REG32(chip, REG_BUS) |= 0x02;
		break;
	default:
		SD_REG32(chip, REG_BUS) |= 0x01;
		break;
	}
}

static int ftsdc010_init(struct mmc *mmc)
{
	ftsdc010_chip_t *chip = mmc->priv;

	if (SD_REG32(chip, REG_STR) & STR_CARD_REMOVED)
		return NO_CARD_ERR;

	if (SD_REG32(chip, REG_STR) & STR_WPROT) {
		printf("ftsdc010: write protected\n");
		chip->wprot = 1;
	}

	chip->fifo = (SD_REG32(chip, REG_FEA) & 0xFF) << 2;

	/* 1. chip reset */
	SD_REG32(chip, REG_CMD) = CMD_RST;
	while(SD_REG32(chip, REG_CMD) & CMD_RST);

	/* 2. enter low speed mode (400k card detection) */
	ftsdc010_clkset(chip, 400000);

	/* 3. interrupt disabled */
	SD_REG32(chip, REG_IMR) = 0;

	return 0;
}

int ftsdc010_mmc_init(int devid)
{
	struct mmc *mmc = NULL;
	ftsdc010_chip_t *chip = NULL;

	mmc = malloc(sizeof(struct mmc));
	if (!mmc)
		return -ENOMEM;
	memset(mmc, 0, sizeof(struct mmc));

	chip = malloc(sizeof(ftsdc010_chip_t));
	if (!chip) {
		free(mmc);
		return -ENOMEM;
	}
	memset(chip, 0, sizeof(ftsdc010_chip_t));

	chip->iobase   = (void *)(CONFIG_FTSDC010_BASE + (devid << 20));
	mmc->priv      = chip;

	sprintf(mmc->name, "ftsdc010");
	mmc->send_cmd  = ftsdc010_request;
	mmc->set_ios   = ftsdc010_set_ios;
	mmc->init      = ftsdc010_init;

	switch((SD_REG32(chip, REG_BUS) >> 3) & 3) {
	case 1:
		mmc->host_caps = MMC_MODE_HS | MMC_MODE_HS_52MHz | MMC_MODE_4BIT;
		break;
	case 2:
		mmc->host_caps = MMC_MODE_HS | MMC_MODE_HS_52MHz | MMC_MODE_4BIT | MMC_MODE_8BIT;
		break;
	default:
		mmc->host_caps = MMC_MODE_HS | MMC_MODE_HS_52MHz;
		break;
	}

#ifdef CONFIG_SYS_CLK_FREQ
	chip->sclk = CONFIG_SYS_CLK_FREQ;
#else
	chip->sclk = clk_get_rate("SDC");
#endif

	mmc->voltages  = MMC_VDD_32_33 | MMC_VDD_33_34;
	mmc->f_max     = chip->sclk / 2;
	mmc->f_min     = chip->sclk / 0x100;
	mmc->block_dev.part_type = PART_TYPE_DOS;
#if 1
	printf("ftsdc010: %d, iobase=0x%08x, base clock = %d\n",
		devid, (uint32_t)chip->iobase, chip->sclk);
#endif

	mmc_register(mmc);

	return 0;
}
