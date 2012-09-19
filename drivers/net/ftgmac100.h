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

#ifndef FTGMAC100_H
#define FTGMAC100_H

/*
 *        FTGMAC100 MAC Registers
 */

#define ISR_REG             0x00	/* interrups status register */
#define IMR_REG             0x04	/* interrupt maks register */
#define MAC_MADR_REG        0x08	/* MAC address (Most significant) */
#define MAC_LADR_REG        0x0c	/* MAC address (Least significant) */
#define MAHT0_REG           0x10	/* Multicast Address Hash Table 0 register */
#define MAHT1_REG           0x14	/* Multicast Address Hash Table 1 register */
#define TXPD_REG            0x18	/* Transmit Poll Demand register */
#define RXPD_REG            0x1c	/* Receive Poll Demand register */
#define TXR_BADR_REG        0x20	/* Transmit Ring Base Address register */
#define RXR_BADR_REG        0x24	/* Receive Ring Base Address register */
#define TXRHI_BADR_REG      0x28	/* High Priority Transmit Ring Base Address register */
#define RXRHI_BADR_REG      0x2C	/* High Priority Receive Ring Base Address register */
#define ITC_REG             0x30	/* interrupt timer control register */
#define APTC_REG            0x34	/* Automatic Polling Timer control register */
#define DBLAC_REG           0x38	/* DMA Burst Length and Arbitration control register */

#define MACCR_REG           0x50	/* MAC control register */
#define MACSR_REG           0x54	/* MAC status register */
#define TM_REG              0x58	/* test mode register */
#define PHYCR_REG           0x60	/* PHY control register */
#define PHYDATA_REG         0x64	/* PHY Write Data register */
#define FCR_REG             0x68	/* Flow Control register */
#define BPR_REG             0x6c	/* back pressure register */
#define WOLCR_REG           0x70	/* Wake-On-Lan control register */
#define WOLSR_REG           0x74	/* Wake-On-Lan status register */
#define WFCRC_REG           0x78	/* Wake-up Frame CRC register */
#define WFBM1_REG           0x80	/* wake-up frame byte mask 1st double word register */
#define WFBM2_REG           0x84	/* wake-up frame byte mask 2nd double word register */
#define WFBM3_REG           0x88	/* wake-up frame byte mask 3rd double word register */
#define WFBM4_REG           0x8c	/* wake-up frame byte mask 4th double word register */

/* Interrupt status register(ISR), Interrupt mask register(IMR) bit setting */
#define NOHTXB_bit		(1UL<<10)
#define PHYSTS_CHG_bit		(1UL<<9)
#define AHB_ERR_bit		(1UL<<8)
#define XPKT_LOST_bit		(1UL<<7)
#define NOTXBUF_bit		(1UL<<6)
#define XPKT_OK_bit		(1UL<<5)	/* FIFO */
#define XPKT_FINISH_bit 	(1UL<<4)	/* ETH */
#define RPKT_LOST_bit		(1UL<<3)
#define NORXBUF_bit		(1UL<<2)
#define RPKT_OK_bit		(1UL<<1)	/* FIFO */
#define RPKT_FINISH_bit 	(1UL<<0)	/* ETH */

/* MACC control bits */
#define SW_RST_bit		(1UL<<31)
#define Speed_100_bit		(1UL<<19)
#define CRC_DIS_bit		(1UL<<18)
#define RX_BROADPKT_bit 	(1UL<<17)	/* Receiving broadcast packet */
#define RX_MULTIPKT_bit 	(1UL<<16)	/* receiving multicast packet */
#define HT_MULTI_EN_bit 	(1UL<<15)
#define RCV_ALL_bit		(1UL<<14)	/* not check incoming packet's destination address */
#define JUMBO_LF_bit		(1UL<<13)
#define RX_RUNT_bit		(1UL<<12)	/* Store incoming packet even its length is les than 64 byte */
#define CRC_APD_bit		(1UL<<10)	/* append crc to transmit packet */
#define GMODE_bit		(1UL<<9)
#define FULLDUP_bit		(1UL<<8)	/* full duplex */
#define ENRX_IN_HALFTX_bit	(1UL<<7)	/* rx in half tx */
#define LOOP_EN_bit		(1UL<<6)	/* Internal loop-back */
#define HPTXR_EN_bit		(1UL<<5)	/* High Priority Tx Ring */
#define VLAN_RM_bit		(1UL<<4)	/* VLAN Tag Removal */
#define RCV_EN_bit		(1UL<<3)	/* receiver enable */
#define XMT_EN_bit		(1UL<<2)	/* transmitter enable */
#define RDMA_EN_bit		(1UL<<1)	/* enable DMA receiving channel */
#define XDMA_EN_bit		(1UL<<0)	/* enable DMA transmitting channel */

/*
 *        MII PHY Registers
 */

/*
 * Bits related to the MII interface
 */
#define MIIREG_READ		(1 << 26)
#define MIIREG_WRITE		(1 << 27)
#define MIIREG_PHYREG_SHIFT	21
#define MIIREG_PHYADDR_SHIFT	16

/*
 *        Receive Ring descriptor structure
 */
typedef struct
{
	/* RXDES0 */
	uint32_t len: 14;
	uint32_t rsvd1: 1;
	uint32_t end: 1;
	uint32_t mcast: 1;
	uint32_t bcast: 1;
#if 1
	uint32_t error: 5;
#else
	uint32_t rxerr: 1;
	uint32_t crcerr: 1;
	uint32_t ftl: 1;
	uint32_t runt: 1;
	uint32_t oddnb: 1;
#endif
	uint32_t fifofull: 1;
	uint32_t pauseopc: 1;
	uint32_t pausefrm: 1;
	uint32_t rsvd2: 2;
	uint32_t lrs: 1;
	uint32_t frs: 1;
	uint32_t rsvd3: 1;
	uint32_t owner: 1; /* 31 - 1:Software, 0: Hardware */

	/* RXDES1 */
	uint32_t vlantag: 16;
	uint32_t rsvd4: 4;
	uint32_t proto: 2;
	uint32_t llc: 1;
	uint32_t df: 1;
	uint32_t vlan: 1;
	uint32_t tcpcs: 1;
	uint32_t udpcs: 1;
	uint32_t ipcs: 1;
	uint32_t rsvd5: 4;

	/* RXDES2 */
	uint32_t vbuf;

	/* RXDES3 */
	uint32_t buf;
} RX_DESC;

typedef struct
{
	/* TXDES0 */
	uint32_t len: 14;
	uint32_t rsvd1: 1;
	uint32_t end: 1;
	uint32_t rsvd2: 3;
	uint32_t crcerr: 1;
	uint32_t rsvd3: 8;
	uint32_t lts: 1;
	uint32_t fts: 1;
	uint32_t rsvd4: 1;
	uint32_t owner: 1; /* 31 - 1:Hardware, 0: Software */

	/* TXDES1 */
	uint32_t vlantag: 16;
	uint32_t vlan: 1;
	uint32_t tcpcs: 1;
	uint32_t udpcs: 1;
	uint32_t ipcs: 1;
	uint32_t rsvd5: 2;
	uint32_t llc: 1;
	uint32_t rsvd6: 7;
	uint32_t tx2fic: 1;
	uint32_t txic: 1;

	/* TXDES2 */
	uint32_t vbuf;

	/* TXDES3 */
	uint32_t buf;
} TX_DESC;
#endif  /* FTGMAC100_H */
