/*
 * (C) Copyright 2012 Faraday Technology
 * Bing-Yao,Luo <bjluo@faraday-tech.com>
 *
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
#include <command.h>
#include <malloc.h>
#include <net.h>
#include <miiphy.h>
#include <asm/io.h>
#include <asm/arch/dma-mapping.h>

#include "ftgmac100.h"

#define DEBUG_DESC		0
#define DEBUG_TX		0
#define DEBUG_RX		0

#define CFG_RXDES_NUM	8
#define CFG_TXDES_NUM	2
#define CFG_XBUF_SIZE	1536

struct ftgmac100_priv
{
	uint32_t		iobase;
	uint32_t		irqmask;
	uint32_t		maccr;
	uint32_t		lnkup;

	volatile RX_DESC	*rx_descs;
	uint32_t		rx_descs_dma;
	uint32_t		rx_idx;

	volatile TX_DESC	*tx_descs;
	uint32_t		tx_descs_dma;
	uint32_t		tx_idx;

	uint32_t		phy_addr;
	struct {
		uint32_t	oui   : 24;
		uint32_t	model :  6;
		uint32_t	rsvd  : 30;
		uint32_t	rev   :  4;
	} phy_id;
};

#define EMAC_REG32(dev, off)	*(volatile uint32_t *)((dev)->iobase + (off))

DECLARE_GLOBAL_DATA_PTR;

static char ftgmac100_mac_addr[] = { 0x00, 0x41, 0x71, 0x00, 0x00, 0x52 };

static int ftgmac100_reset(struct eth_device *dev);

static uint16_t mdio_read(struct eth_device *dev, uint8_t phyaddr, uint8_t phyreg)
{
	uint32_t tmp;

	tmp = MIIREG_READ
	      | (phyaddr << MIIREG_PHYADDR_SHIFT)
	      | (phyreg  << MIIREG_PHYREG_SHIFT)
	      | 0x3000003F;

	EMAC_REG32(dev, PHYCR_REG) = tmp;

	do {
		tmp = EMAC_REG32(dev, PHYCR_REG);
	} while(tmp & MIIREG_READ);

	tmp = EMAC_REG32(dev, PHYDATA_REG);

	return (uint16_t)(tmp >> 16);
}

static void mdio_write(struct eth_device *dev, uint8_t phyaddr, uint8_t phyreg, uint16_t phydata)
{
	unsigned int tmp;

	tmp = MIIREG_WRITE
	      | (phyaddr << MIIREG_PHYADDR_SHIFT)
	      | (phyreg  << MIIREG_PHYREG_SHIFT)
	      | 0x3000003F;

	EMAC_REG32(dev, PHYDATA_REG) = phydata;
	EMAC_REG32(dev, PHYCR_REG)   = tmp;

	do {
		tmp = EMAC_REG32(dev, PHYCR_REG);
	} while(tmp & MIIREG_WRITE);
}

static uint32_t ftgmac100_phyqry(struct eth_device *dev)
{
	uint32_t retry, maccr;
	uint16_t pa, tmp, nego;
	struct ftgmac100_priv *priv = dev->priv;

	maccr = Speed_100_bit | FULLDUP_bit;

	/* 0. find the phy device  */
	for (pa = 0; pa < 32; ++pa) {
		tmp = mdio_read(dev, pa, MII_PHYSID1);
		if (tmp == 0xFFFF || tmp == 0x0000)
			continue;
		break;
	}
	if (pa >= 32) {
		puts("ftgmac100: phy device not found!\n");
		return maccr;
	} else {
		priv->phy_addr = pa;
	}

	/* 1. check link status */
	for (retry = 0x300000; retry; --retry) {
		tmp = mdio_read(dev, priv->phy_addr, MII_BMSR);
		if (tmp & BMSR_LSTATUS)
			break;
	}
	if (!retry) {
		priv->lnkup = 0;
		puts("ftgmac100: link down\n");
		goto exit;
	}
	priv->lnkup = 1;

	/* 2. check A/N status */
	for (retry = 0x300000; retry; --retry) {
		tmp = mdio_read(dev, priv->phy_addr, MII_BMSR);
		if (tmp & BMSR_ANEGCOMPLETE)
			break;
	}
	if (!retry) {
		puts("ftgmac100: A/N failed\n");
		goto exit;
	}

	/* 3. build MACCR with the PHY status */
	maccr = 0;
	tmp = mdio_read(dev, priv->phy_addr, MII_PHYSID2);
	priv->phy_id.oui   = (mdio_read(dev, priv->phy_addr, MII_PHYSID1 ) << 6) |(tmp >> 10);
	priv->phy_id.model = (tmp >> 4) & 0x3F;
	priv->phy_id.rev   = tmp & 0x0F;

	/* 3-1. duplex/speed detect */
	nego  = mdio_read(dev, priv->phy_addr, MII_ADVERTISE);
	nego &= mdio_read(dev, priv->phy_addr, MII_LPA);
	if (nego & LPA_100FULL)
		maccr |= Speed_100_bit | FULLDUP_bit;
	else if (nego & LPA_100HALF)
		maccr |= Speed_100_bit | ENRX_IN_HALFTX_bit;
	else if (nego & LPA_10FULL)
		maccr |= FULLDUP_bit;
	else if (nego & LPA_10HALF)
		maccr |= ENRX_IN_HALFTX_bit;

	/* 3-2. G-MODE detection routines */
	switch(priv->phy_id.oui) {
	case 0x005043:	/* Marvell 88E1111 */
		tmp = mdio_read(dev, priv->phy_addr, 17);
		if (!(tmp & (1 << 11))) {
			printf("ftgmac100: speed & duplex are un-resolved?\n");
			break;
		}
		if (!(tmp & (1 << 10))) {
			printf("ftgmac100: link down?\n");
			break;
		}
		switch ((tmp >> 14) &0x03) {
			case 0:	/* 10 Mbps */
			case 1:	/* 100 Mbps */
				break;
			case 2:	/* 1000 Mbps */
				maccr |= GMODE_bit;
				break;
		}
		if (tmp & (1 << 13)) {	/* full */
			maccr &= ~ENRX_IN_HALFTX_bit;
			maccr |= FULLDUP_bit;
		} else {				/* half */
			maccr &= ~FULLDUP_bit;
			maccr |= ENRX_IN_HALFTX_bit;
		}
		break;

	default:
		tmp  = mdio_read(dev, priv->phy_addr, MII_BMSR);
		if (!(tmp & BMSR_ESTATEN))
			break;
		tmp  = mdio_read(dev, priv->phy_addr, MII_ESTATUS);
		if (tmp & (ESTATUS_1000_TFULL | ESTATUS_1000_THALF)) {
			uint16_t ctrl1000, stat1000;
			ctrl1000 = mdio_read(dev, priv->phy_addr, MII_CTRL1000);
			stat1000 = mdio_read(dev, priv->phy_addr, MII_STAT1000);
			printf("ftgmac100: gmii detected(0x%04X, 0x%04X)\n", ctrl1000, stat1000);
			if ((ctrl1000 & ADVERTISE_1000FULL) && (stat1000 & LPA_1000FULL)) {
				maccr |= GMODE_bit | FULLDUP_bit;
				maccr &= ~ENRX_IN_HALFTX_bit;
			} else if ((ctrl1000 & ADVERTISE_1000HALF) && (stat1000 & LPA_1000HALF)) {
				maccr |= GMODE_bit | ENRX_IN_HALFTX_bit;
				maccr &= ~FULLDUP_bit;
			}
		}
		break;
	}

exit:
	printf("ftgmac100: PHY OUI=0x%06X, Model=0x%02X, Rev.=0x%02X\n",
		priv->phy_id.oui, priv->phy_id.model, priv->phy_id.rev);

	printf("ftgmac100: %d Mbps, %s\n",
		(maccr & GMODE_bit) ? 1000 : ((maccr & Speed_100_bit) ? 100 : 10),
		(maccr & FULLDUP_bit) ? "Full" : "half");

	return maccr;
}

static int ftgmac100_reset(struct eth_device *dev)
{
	uint32_t i, tmp, maccr;
	struct ftgmac100_priv	*priv = dev->priv;

	/*
	 * 1. MAC reset
	 */
	EMAC_REG32(priv, MACCR_REG) = SW_RST_bit;
	while (1) {
		tmp = EMAC_REG32(priv, MACCR_REG);
		if ( !(tmp & SW_RST_bit) )
			break;
	}

	/* 1-1. tx ring */
	for (i = 0; i < CFG_TXDES_NUM; ++i) {
		priv->tx_descs[i].owner = 0;	/* owned by SW */
	}
	priv->tx_idx = 0;

	/* 1-2. rx ring	*/
	for (i = 0; i < CFG_RXDES_NUM; ++i) {
		priv->rx_descs[i].owner = 0;	/* owned by HW */
	}
	priv->rx_idx = 0;

	/*
	 * 2. PHY status query
	 */
	maccr = ftgmac100_phyqry(dev);

	/* 3. Fix up the MACCR value */
	priv->maccr   = maccr | CRC_APD_bit | RCV_ALL_bit | RX_RUNT_bit | RCV_EN_bit | XMT_EN_bit | RDMA_EN_bit | XDMA_EN_bit;
	priv->irqmask = 0;

	/* 4. MAC address setup */
	do {
		uint8_t *ma = dev->enetaddr;
		EMAC_REG32(priv, MAC_MADR_REG) = ma[1] | (ma[0] << 8);
		EMAC_REG32(priv, MAC_LADR_REG) = ma[5] | (ma[4] << 8) | (ma[3] << 16) | (ma[2] << 24);
	} while(0);

	/* 5. MAC registers setup */
	EMAC_REG32(priv, RXR_BADR_REG) = priv->rx_descs_dma;
	EMAC_REG32(priv, TXR_BADR_REG) = priv->tx_descs_dma;
	EMAC_REG32(priv, ITC_REG)      = 0x00001010;
	EMAC_REG32(priv, APTC_REG)     = 0x00000001;
	EMAC_REG32(priv, DBLAC_REG)    = 0x00022F72;
	EMAC_REG32(priv, ISR_REG)      = 0x000007FF;
	EMAC_REG32(priv, IMR_REG)      = priv->irqmask;
	EMAC_REG32(priv, MACCR_REG)    = priv->maccr;

	return 0;
}

static int ftgmac100_probe(struct eth_device *dev, bd_t *bis)
{
	puts("ftgmac100: probe\n");

	if (ftgmac100_reset(dev))
		return -1;

	return 0;
}

static void ftgmac100_halt(struct eth_device *dev)
{
	struct ftgmac100_priv *priv = dev->priv;

	EMAC_REG32(priv, IMR_REG) = 0;
	EMAC_REG32(priv, MACCR_REG)	= 0;

	puts("ftgmac100: halt\n");
}

static int ftgmac100_send(struct eth_device *dev, volatile void *packet, int length)
{
	struct ftgmac100_priv *priv = dev->priv;
	volatile TX_DESC *cur_desc;

	if (!priv->lnkup)
		return 0;
#if DEBUG_TX
	printf("ftgmac100: tx@0x%08X, len=%d\n", (uint32_t)packet, length);
#endif	/* #ifdef DEBUG_TX */

	if (length <= 0 || length > CFG_XBUF_SIZE) {
		printf("ftgmac100: bad tx packet length(%d)\n", length);
		return 0;
	}

	if (length < 64)
		length = 64;

	cur_desc = &priv->tx_descs[priv->tx_idx];
	if (cur_desc->owner) {
		EMAC_REG32(priv, TXPD_REG) = 0xffffffff;	/* kick-off Tx DMA */
		printf("ftgmac100: out of txd\n");
		return 0;
	}

	memcpy((void *)cur_desc->vbuf, (void *)packet, length);
	dma_map_single((void *)cur_desc->vbuf, length, DMA_TO_DEVICE);

	cur_desc->len   = length;
	cur_desc->lts   = 1;
	cur_desc->fts   = 1;
	cur_desc->owner = 1;

	EMAC_REG32(priv, TXPD_REG) = 0xffffffff;	/* kick-off Tx DMA */

#if DEBUG_TX
	printf("ftgmac100: txd[%d]@0x%08X, buf=0x%08X(pa=0x%08X)\n",
	       priv->tx_idx, (uint32_t)cur_desc,
	       cur_desc->vbuf, cur_desc->buf);
#endif

	priv->tx_idx = (priv->tx_idx + 1) % CFG_TXDES_NUM;

	return length;
}

static int ftgmac100_recv(struct eth_device *dev)
{
	struct ftgmac100_priv *priv = dev->priv;
	volatile RX_DESC *cur_desc;
	uint32_t rlen = 0;

	if (!priv->lnkup)
		return 0;

	do {
		uint8_t *buf;
		uint16_t len;

		cur_desc = &priv->rx_descs[priv->rx_idx];
		if (cur_desc->owner == 0)
			break;

		len = cur_desc->len;
		buf = (uint8_t *)cur_desc->vbuf;

		if (cur_desc->error) {
			printf("ftgmac100: rx error\n");
		} else {
#if DEBUG_RX
			printf("ftgmac100: rx@0x%08X, len=%d\n", (uint32_t)buf, len);
#endif
			dma_map_single(buf, len, DMA_FROM_DEVICE);
			NetReceive(buf, len);
			rlen += len;
		}

		cur_desc->len   = CFG_XBUF_SIZE;
		cur_desc->owner = 0;	/* owned by hardware */

		priv->rx_idx = (priv->rx_idx + 1) % CFG_RXDES_NUM;

	} while(0);

	return rlen;
}

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)

static int ftgmac100_mii_read (const char *devname, uint8_t addr, uint8_t reg, uint16_t *value)
{
	int ret = 0;
	struct eth_device *dev;

	dev = eth_get_dev_by_name(devname);
	if (dev == NULL) {
		printf("%s: no such device\n", devname);
		ret = -1;
	} else {
		*value = mdio_read(dev, addr, reg);
	}

	return ret;
}

static int ftgmac100_mii_write (const char *devname, uint8_t addr, uint8_t reg, uint16_t value)
{
	int ret = 0;
	struct eth_device *dev;

	dev = eth_get_dev_by_name(devname);
	if (dev == NULL) {
		printf("%s: no such device\n", devname);
		ret = -1;
	} else {
		mdio_write(dev, addr, reg, value);
	}

	return ret;
}

#endif	/* #if defined(CONFIG_MII) || defined(CONFIG_CMD_MII) */

int ftgmac100_initialize(bd_t *bis)
{
	int i, card_number = 0;
	char enetvar[32];
	char *tmp, *end;
	struct eth_device *dev;
	struct ftgmac100_priv *priv;

	dev = (struct eth_device *)malloc(sizeof(struct eth_device) + sizeof(struct ftgmac100_priv));
	if (dev == NULL) {
		panic("ftgmac100: out of memory 1\n");
		return -1;
	}
	priv = (struct ftgmac100_priv *)(dev + 1);
	memset(dev, 0, sizeof(struct eth_device) + sizeof(struct ftgmac100_priv));

	sprintf(dev->name, "FTGMAC#%d", card_number);

	dev->iobase = priv->iobase = CONFIG_FTGMAC100_BASE;
	printf("ftgmac100: ioaddr = 0x%08x\n", (u32) priv->iobase);
	dev->priv = priv;
	dev->init = ftgmac100_probe;
	dev->halt = ftgmac100_halt;
	dev->send = ftgmac100_send;
	dev->recv = ftgmac100_recv;

	sprintf(enetvar, card_number ? "eth%daddr" : "ethaddr", card_number);

	tmp = getenv (enetvar);
	if (tmp != NULL) {
		for (i = 0; i < 6; i++) {
			dev->enetaddr[i] = tmp ? simple_strtoul(tmp, &end, 16) : 0;
			if (tmp)
				tmp = (*end) ? end + 1 : end;
		}
	} else {
		memcpy(dev->enetaddr, ftgmac100_mac_addr, 6);
	}

	/*
	 * init tx descriptors (it must be 16 bytes aligned)
	 */
	priv->tx_descs = dma_alloc_coherent(sizeof(TX_DESC)*CFG_TXDES_NUM, (unsigned long *)&priv->tx_descs_dma);
	if (priv->tx_descs == NULL)
		panic("ftgmac100: out of memory 3\n");
	memset((void *)priv->tx_descs, 0, sizeof(TX_DESC) * CFG_TXDES_NUM);
	for (i = 0; i < CFG_TXDES_NUM; ++i) {
		size_t align = 32;
		uint8_t *va = memalign(align, CFG_XBUF_SIZE);
		if (va == NULL)
			panic("ftgmac100: out of memory 4\n");
		priv->tx_descs[i].vbuf  = (uint32_t)va;
		priv->tx_descs[i].buf   = virt_to_phys(va);
		priv->tx_descs[i].len   = 0;
		priv->tx_descs[i].end   = 0;
		priv->tx_descs[i].owner = 0;	/* owned by SW */
	}
	priv->tx_descs[CFG_TXDES_NUM - 1].end = 1;
	priv->tx_idx = 0;

	/*
	 * init rx descriptors
	 */
	priv->rx_descs = dma_alloc_coherent(sizeof(RX_DESC)*CFG_RXDES_NUM, (unsigned long *)&priv->rx_descs_dma);
	if (priv->rx_descs == NULL)
		panic("ftgmac100: out of memory 4\n");
	memset((void *)priv->rx_descs, 0, sizeof(RX_DESC) * CFG_RXDES_NUM);
	for (i = 0; i < CFG_RXDES_NUM; ++i) {
		size_t align = 32;
		uint8_t *va = memalign(align, CFG_XBUF_SIZE);
		if (va == NULL)
			panic("ftgmac100: out of memory 5\n");
		priv->rx_descs[i].vbuf  = (uint32_t)va;
		priv->rx_descs[i].buf   = virt_to_phys(va);
		priv->rx_descs[i].len   = CFG_XBUF_SIZE;
		priv->rx_descs[i].end   = 0;
		priv->rx_descs[i].owner = 0;	/* owned by HW */
	}
	priv->rx_descs[CFG_RXDES_NUM - 1].end = 1;
	priv->rx_idx = 0;

	eth_register (dev);

#if defined(CONFIG_MII) || defined(CONFIG_CMD_MII)
	miiphy_register (dev->name, ftgmac100_mii_read, ftgmac100_mii_write);
#endif

#if DEBUG_DESC
	for (i = 0; i < CFG_TXDES_NUM; ++i) {
		printf("TXD[%d]: 0x%08X, 0x%08X<=0x%08x\n",
			i, (uint32_t)&priv->tx_descs[i],
			(uint32_t)priv->tx_descs[i].vbuf,
			(uint32_t)priv->tx_descs[i].buf);
	}
	for (i = 0; i < CFG_RXDES_NUM; ++i) {
		printf("RXD[%d]: 0x%08X, 0x%08X<=0x%08X\n",
			i, (uint32_t)&priv->rx_descs[i],
			(uint32_t)priv->rx_descs[i].vbuf,
			(uint32_t)priv->rx_descs[i].buf);
	}
#endif

	card_number++;

	return card_number;
}
