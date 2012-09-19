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
#include <nand.h>
#include <malloc.h>

#include <asm/arch/bits.h>
#include "ftnandc021.h"

typedef struct ftnandc021_info
{
	void             *iobase;
	unsigned int      cmd;

	unsigned int      pgidx;

	unsigned int      off;
	uint8_t           buf[256];

	unsigned int      adrc;	/* address cycle */
	unsigned int      pgsz;	/* page size */
	unsigned int      bksz;	/* block size */
} ftnandc021_info_t;

/* Register access macros */
#define NAND_REG32(priv, off) \
	*(volatile uint32_t *)((uint32_t)((priv)->iobase) + (off))

static struct nand_ecclayout ftnandc021_oob_2k = {
	.eccbytes = 24,
	.eccpos = {
		   40, 41, 42, 43, 44, 45, 46, 47,
		   48, 49, 50, 51, 52, 53, 54, 55,
		   56, 57, 58, 59, 60, 61, 62, 63},
	.oobfree = {
		{.offset = 9,
		 .length = 3}}
};

static int
ftnandc021_reset(struct nand_chip *chip)
{
	ftnandc021_info_t *priv = chip->priv;
	uint32_t bk = 2;	/* 64 pages */
	uint32_t pg = 1;	/* 2k */
	uint32_t ac = 2;	/* 5 */

	uint32_t mask = NANDC_NANDC_SW_RESET | NANDC_BMC_SW_RESET | NANDC_ECC_SW_RESET;

#ifdef CONFIG_FTNANDC021_ACTIMING_1
	NAND_REG32(priv, REG_AC1_CONTROL)   = CONFIG_FTNANDC021_ACTIMING_1;
#endif
#ifdef CONFIG_FTNANDC021_ACTIMING_2
	NAND_REG32(priv, REG_AC2_CONTROL)   = CONFIG_FTNANDC021_ACTIMING_2;
#endif

	NAND_REG32(priv, REG_INT_EN)        = 0;
	NAND_REG32(priv, REG_PAGE_INDEX)    = 0;
	NAND_REG32(priv, REG_WRITE_BI)      = 0xff;
	NAND_REG32(priv, REG_WRITE_LSN_CRC) = 0xffffffff;
	if (chip->options & NAND_BUSWIDTH_16)
		NAND_REG32(priv, REG_FLOW_CONTROL) = BIT8 | BIT7 | NANDC_IO_WIDTH_16BIT;
	else
		NAND_REG32(priv, REG_FLOW_CONTROL) = BIT8 | BIT7 | NANDC_IO_WIDTH_8BIT;

	/* chip reset */
	NAND_REG32(priv, REG_MLC_SW_RESET)  = mask;

	/* wait until chip ready */
	while(NAND_REG32(priv, REG_MLC_SW_RESET) & BIT0);

	switch (priv->bksz / priv->pgsz) {
	case 16:
		bk = 0;
		break;
	case 32:
		bk = 1;
		break;
	case 64:
		bk = 2;
		break;
	case 128:
		bk = 3;
		break;
	}

	switch (priv->pgsz) {
	case 512:
		pg = 0;
		break;
	case 2048:
		pg = 1;
		break;
	case 4096:
		pg = 2;
		break;
	}

	switch (priv->adrc) {
	case 3:
		ac = 0;
		break;
	case 4:
		ac = 1;
		break;
	case 5:
		ac = 2;
		break;
	}

	NAND_REG32(priv, REG_MEMORY_CONFIG) = NANDC_MS_32GB|NANDC_MOD0_ENABLE|(bk << 16)|(pg << 8)|(ac << 10);

	/* PIO mode */
	NAND_REG32(priv, REG_BMC_PIO_MODE_READY) = 0;

	return 0;
}

static inline int
ftnandc021_ckst(ftnandc021_info_t *priv)
{
	int rc = 0;
	uint32_t st = NAND_REG32(priv, REG_DEV_ID_BYTE47);

	if (st & BIT0) {
		printf("ftnandc021: status failed\n");
		rc = -1;
	}

	if( !(st & BIT6) ) {
		printf("ftnandc021: not ready\n");
		rc = -1;
	}

	if( !(st & BIT7) ) {
		printf("ftnandc021: write protected\n");
		rc = -1;
	}

	return rc;
}

static inline int
ftnandc021_wait(ftnandc021_info_t *priv)
{
	uint32_t timeout;

	for (timeout = 0x7FFFFF; timeout > 0; --timeout) {
		if (!(NAND_REG32(priv, REG_ACCESS_CONTROL) & NANDC_CMD_LAUNCH))
			break;
	}

	return timeout > 0 ? 0 : -1;
}

static int
ftnandc021_command(ftnandc021_info_t *priv, uint32_t cmd)
{
	int rc = 0;

	NAND_REG32(priv, REG_ACCESS_CONTROL) = NANDC_CMD_LAUNCH | (cmd << NANDC_CMD_OFFSET);

	/*
	 * pgread    : (We have queued data at the IO port)
	 * pgwrite   : nand_ckst (We have queued data at the IO port)
	 * bkerase   : nand_wait + nand_ckst
	 * oobwr     : nand_wait + nand_ckst
	 * otherwise : nand_wait
	 */
	switch (cmd) {
	case NANDC_CMD_PAGE_READ:
		break;
	case NANDC_CMD_PAGE_WRITE_RS:
		rc = ftnandc021_ckst(priv);
		break;
	case NANDC_CMD_BLK_ERASE_RS:
	case NANDC_CMD_SPARE_WRITE_RS:
		rc = ftnandc021_wait(priv) || ftnandc021_ckst(priv);
		break;
	default:
		rc = ftnandc021_wait(priv);
	}

	return rc;
}

/*
 * Check hardware register for wait status. Returns 1 if device is ready,
 * 0 if it is still busy.
 */
static int
ftnandc021_dev_ready(struct mtd_info *mtd)
{
	struct nand_chip  *chip = mtd->priv;
	ftnandc021_info_t *priv = chip->priv;
	int rc = 1;

	if (ftnandc021_wait(priv) || ftnandc021_ckst(priv))
		rc = 0;

	return rc;
}

static void
ftnandc021_read_oob(struct mtd_info *mtd, uint8_t * buf, int len)
{
	struct nand_chip  *chip = mtd->priv;
	ftnandc021_info_t *priv = chip->priv;
	uint32_t tmp;

	/*
	 * Bad Block Information:
	 * 1. Large Page(2048, 4096): off=0, len=2
	 * 2. Small Page(512): off=5, len=1
	*/
	buf[0]  = NAND_REG32(priv, REG_READ_BI) & 0xFF;
	buf[1]  = 0xFF;

	tmp     = NAND_REG32(priv, REG_READ_LSN_CRC);
	buf[8]  = (tmp >>  0) & 0xFF;
	buf[9]  = (tmp >>  8) & 0xFF;
	if (mtd->writesize >=  4096) {
	    buf[12] = (tmp >> 16) & 0xFF;
	    buf[13] = (tmp >> 24) & 0xFF;
	}

	tmp     = NAND_REG32(priv, REG_READ_LSN);
	buf[10] = (tmp >>  0) & 0xFF;
	buf[11] = (tmp >>  8) & 0xFF;
	if (mtd->writesize >=  4096) {
	    buf[14] = (tmp >> 16) & 0xFF;
	    buf[15] = (tmp >> 24) & 0xFF;
	}

	/*
	do {
		uint32_t v = *(uint32_t *)(buf + 8);
		if (v != 0xFFFFFFFF && v != 0xFFFF0000)	{
			printf("ftnandc021: pg=%d, oob=0x%08X, buf@0x%p (%p)\n", priv->pgidx, v, buf, priv->chip.buffers);
		}
	} while(0);
	*/
}

static void
ftnandc021_write_oob(struct mtd_info *mtd, const uint8_t * buf, int len)
{
	struct nand_chip  *chip = mtd->priv;
	ftnandc021_info_t *priv = chip->priv;
	uint32_t tmp;

	NAND_REG32(priv, REG_WRITE_BI) = buf[0];

	tmp = buf[8] | (buf[9] << 8);
	if (mtd->writesize >=  4096)
		tmp |= (buf[12] << 16) | (buf[13] << 24);
	NAND_REG32(priv, REG_WRITE_LSN_CRC) = tmp;

	tmp = buf[10] | (buf[11] << 8);
	if (mtd->writesize >=  4096)
		tmp |= (buf[14] << 16) | (buf[15] << 24);
	NAND_REG32(priv, REG_LSN_CONTROL)   = tmp;
}

static uint8_t
ftnandc021_read_byte(struct mtd_info *mtd)
{
	struct nand_chip  *chip = mtd->priv;
	ftnandc021_info_t *priv = chip->priv;
	uint8_t rc = 0xFF;

	switch(priv->cmd) {
	case NAND_CMD_READID:
	case NAND_CMD_READOOB:
		rc = priv->buf[priv->off % 256];
		priv->off += 1;
		break;
	case NAND_CMD_STATUS:
		rc = (uint8_t)(NAND_REG32(priv, REG_DEV_ID_BYTE47) & 0xFF);
		break;
	default:
		printf("ftnandc021_read_byte...not supported cmd(0x%02X)!?\n", priv->cmd);
		break;
	}

	return rc;
}

static uint16_t
ftnandc021_read_word(struct mtd_info *mtd)
{
	uint16_t rc = 0xFFFF;
	uint8_t *buf= (uint8_t *)&rc;

	buf[0] = ftnandc021_read_byte(mtd);
	buf[1] = ftnandc021_read_byte(mtd);

	return rc;
}

/**
 * Read data from NAND controller into buffer
 * @mtd: MTD device structure
 * @buf: buffer to store date
 * @len: number of bytes to read
 */
static void
ftnandc021_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	struct nand_chip  *chip = mtd->priv;
	ftnandc021_info_t *priv = chip->priv;
	uint32_t off;

	if ((uint32_t)buf & 0x03) {
		printf("ftnandc021_read_buf: buf@0x%08X is not aligned\n", (uint32_t)buf);
		return;
	}

	/* 1. oob read */
	if (len <= mtd->oobsize) {
		ftnandc021_read_oob(mtd, buf, len);
		return;
	}

	/* 3. page read */
	for (off = 0; len > 0; len -= 4, off += 4) {
		ulong ts = get_timer(0);
		do {
			if (NAND_REG32(priv, 0x208) & BIT0)
				break;
		} while(get_timer(ts) < CONFIG_SYS_HZ);

		if (!(NAND_REG32(priv, 0x208) & BIT0)) {
			printf("ftnandc021_read_buf: data timeout (cmd=0x%02X)\n", priv->cmd);
			return;
		}
		*(uint32_t *)(buf + off) = NAND_REG32(priv, REG_BMC_DATA_PORT);
	}

	if (ftnandc021_wait(priv)) {
		printf("ftnandc021_read_buf: command timeout (cmd=0x%02X)\n", priv->cmd);
	}
}

/**
 * Write buffer to NAND controller
 * @mtd: MTD device structure
 * @buf: data buffer
 * @len: number of bytes to write
 */
static void
ftnandc021_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	struct nand_chip  *chip = mtd->priv;
	ftnandc021_info_t *priv = chip->priv;
	uint32_t off;

	if ((uint32_t)buf & 0x03) {
		printf("ftnandc021_write_buf: buf@0x%08X is not aligned\n", (uint32_t)buf);
		return;
	}

	/* 1. oob write */
	if (len <= mtd->oobsize)
		return;

	/* 2. page write */
	for (off = 0; len > 0; len -= 4, off += 4) {
		while(!(NAND_REG32(priv, 0x208) & BIT0));
		NAND_REG32(priv, REG_BMC_DATA_PORT) = *(uint32_t *)(buf + off);
	}

	/* 3. wait until command finish */
	if (ftnandc021_wait(priv)) {
		printf("ftnandc021_write_buf: write fail\n");
	}
}

/**
 * Verify chip data against buffer
 * @mtd: MTD device structure
 * @buf: buffer containing the data to compare
 * @len: number of bytes to compare
 */
static int
ftnandc021_verify_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	int rc = 0;
	uint8_t *tmp;

	len = min_t(int, len, mtd->writesize);
	tmp = malloc(mtd->writesize);

	if (tmp == NULL) {
		printf("ftnandc021_verify_buf: out of memory\n");
		return -1;
	} else {
		ftnandc021_read_buf(mtd, tmp, len);
		if (memcmp(tmp, buf, len))
			rc = -2;
	}

	free(tmp);

	if (rc)
		printf("ftnandc021_verify_buf...failed (buf@%p, len=%d)\n", buf, len);
#if 0
	else
		printf("ftnandc021_verify_buf...ok (bksz=%d)\n", mtd->writesize);
#endif

	return rc;
}

static void
ftnandc021_cmdfunc(struct mtd_info *mtd, unsigned cmd, int column, int pgidx)
{
	struct nand_chip  *chip = mtd->priv;
	ftnandc021_info_t *priv = chip->priv;

	priv->cmd   = cmd;
	priv->pgidx = pgidx;

	switch (cmd) {
	case NAND_CMD_READID:	/* 0x90 */
		if (ftnandc021_command(priv, NANDC_CMD_READ_ID)) {
			printf("ftnandc021_cmdfunc: RDID failed.\n");
		} else {
			uint32_t tmp;
			tmp = NAND_REG32(priv, REG_DEV_ID_BYTE03);
			memcpy(priv->buf + 0, &tmp, 4);
			tmp = NAND_REG32(priv, REG_DEV_ID_BYTE47);
			memcpy(priv->buf + 4, &tmp, 4);
			priv->off = 0;
			/*
			printf("ftnandc021_cmdfunc: ID= 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n",
				priv->buf[0], priv->buf[1], priv->buf[2], priv->buf[3], priv->buf[4] );
			*/
		}
		break;
	case NAND_CMD_READ0:	/* 0x00 */
		NAND_REG32(priv, REG_PAGE_INDEX) = pgidx;
		NAND_REG32(priv, REG_PAGE_COUNT) = 1;
		if (ftnandc021_command(priv, NANDC_CMD_PAGE_READ))
			printf("ftnandc021_cmdfunc: PGREAD failed.\n");
		break;
	case NAND_CMD_READOOB:	/* 0x50 */
		NAND_REG32(priv, REG_PAGE_INDEX) = pgidx;
		NAND_REG32(priv, REG_PAGE_COUNT) = 1;
		if (ftnandc021_command(priv, NANDC_CMD_SPARE_READ)) {
			printf("ftnandc021_cmdfunc: OOBREAD failed.\n");
		} else {
			ftnandc021_read_oob(mtd, priv->buf, mtd->oobsize);
			priv->off = 0;
		}
		break;
	case NAND_CMD_ERASE1:	/* 0x60 */
		NAND_REG32(priv, REG_PAGE_INDEX) = pgidx;
		NAND_REG32(priv, REG_PAGE_COUNT) = 1;
		break;
	case NAND_CMD_ERASE2:	/* 0xD0 */
		if (ftnandc021_command(priv, NANDC_CMD_BLK_ERASE_RS)) {
			printf("ftnandc021_cmdfunc: ERASE failed, pgidx=%d\n", pgidx);
		}
		break;
	case NAND_CMD_STATUS:	/* 0x70 */
		if (ftnandc021_command(priv, NANDC_CMD_READ_STS)) {
			printf("ftnandc021_cmdfunc: STREAD failed.\n");
		}
		break;
	case NAND_CMD_SEQIN:	/* 0x80 (Write Stage 1.) */
		ftnandc021_write_oob(mtd, chip->oob_poi, mtd->writesize);

		NAND_REG32(priv, REG_PAGE_INDEX) = pgidx;
		NAND_REG32(priv, REG_PAGE_COUNT) = 1;
		if (column >= mtd->writesize) {
			if (ftnandc021_command(priv, NANDC_CMD_SPARE_WRITE_RS)) {
				printf("ftnandc021_cmdfunc: oob write failed..\n");
			}
		} else {
			if (ftnandc021_command(priv, NANDC_CMD_PAGE_WRITE_RS)) {
				printf("ftnandc021_cmdfunc: page write failed..\n");
			}
		}
		break;
	case NAND_CMD_PAGEPROG:	/* 0x10 (Write Stage 2.) */
		break;
	case NAND_CMD_RESET:	/* 0xFF */
		if (ftnandc021_command(priv, NANDC_CMD_RESET)) {
			printf("ftnandc021_cmdfunc: RESET failed.\n");
		}
		break;
	default:
		printf("ftnandc021_cmdfunc: error, unsupported command (0x%x).\n", cmd);
	}
}

/**
 * hardware specific access to control-lines
 * @mtd: MTD device structure
 * @cmd: command to device
 * @ctrl:
 * NAND_NCE: bit 0 -> don't care
 * NAND_CLE: bit 1 -> Command Latch
 * NAND_ALE: bit 2 -> Address Latch
 *
 * NOTE: boards may use different bits for these!!
 */
static void
ftnandc021_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
}

int
ftnandc021_probe(struct nand_chip *chip)
{
	ftnandc021_info_t *priv;

	priv = malloc(sizeof(ftnandc021_info_t));
	if (!priv)
		return -1;

	memset(priv, 0, sizeof(*priv));
	priv->iobase = (void *)CONFIG_FTNANDC021_BASE;
	priv->adrc   = (unsigned int)chip->priv;
	priv->pgsz   = 1 << chip->page_shift;
	priv->bksz   = 1 << chip->phys_erase_shift;

	chip->priv       = priv;
	chip->cmd_ctrl   = ftnandc021_hwcontrol;
	chip->cmdfunc    = ftnandc021_cmdfunc;
	chip->dev_ready  = ftnandc021_dev_ready;
	chip->chip_delay = 0;

	chip->read_byte  = ftnandc021_read_byte;
	chip->read_word  = ftnandc021_read_word;
	chip->read_buf   = ftnandc021_read_buf;
	chip->write_buf  = ftnandc021_write_buf;
	chip->verify_buf = ftnandc021_verify_buf;

	chip->ecc.mode   = NAND_ECC_NONE;
	chip->ecc.layout = &ftnandc021_oob_2k;

	chip->options    |= NAND_NO_AUTOINCR;

	ftnandc021_reset(chip);

#if 0
	printf("ftnandc021: pg=%dK, bk=%dK, adrc=%d\n",
	       priv->pgsz >> 10, priv->bksz >> 10, priv->adrc);
#endif
	return 0;
}
