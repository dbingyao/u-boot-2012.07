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

#ifndef FTNANDC021_H
#define FTNANDC021_H

/* SMC Register Offset Definitions */
/** ECC control register **/
#define REG_ECC_PARITY0 		0x00
#define REG_ECC_PARITY1 		0x04
#define REG_ECC_PARITY2 		0x08
#define REG_ECC_PARITY3 		0x0C
#define REG_ECC_STATUS			0x10

/** NANDC control register **/
#define REG_HOST_STATUS 		0x100
#define REG_ACCESS_CONTROL		0x104
#define REG_FLOW_CONTROL		0x108
#define REG_PAGE_INDEX			0x10C
#define REG_MEMORY_CONFIG		0x110
#define REG_AC1_CONTROL 		0x114
#define REG_AC2_CONTROL 		0x118
#define REG_TARGET_PAGE_INDEX		0x11C
#define REG_DEV_ID_BYTE03		0x120
#define REG_DEV_ID_BYTE47		0x124
#define REG_INT_EN			0x128
#define REG_INT_STS_CLEAR		0x12C
#define REG_FLASH0_BLK_OFFSET		0x130
#define REG_FLASH1_BLK_OFFSET		0x134
#define REG_FLASH2_BLK_OFFSET		0x138
#define REG_FLASH3_BLK_OFFSET		0x13C
#define REG_WRITE_BI			0x140
#define REG_WRITE_LSN			0x144
#define REG_WRITE_LSN_CRC		0x148
#define REG_LSN_CONTROL 		0x14C
#define REG_READ_BI			0x150
#define REG_READ_LSN			0x154
#define REG_READ_LSN_CRC		0x158

#define REG_F0_CB_TGT_BLK_OFFSET	0x160
#define REG_F1_CB_TGT_BLK_OFFSET	0x164
#define REG_F2_CB_TGT_BLK_OFFSET	0x168
#define REG_F3_CB_TGT_BLK_OFFSET	0x16c

#define REG_F0_LSN_INIT 		0x170
#define REG_F1_LSN_INIT 		0x174
#define REG_F2_LSN_INIT 		0x178
#define REG_F3_LSN_INIT 		0x17C

/** BMC control register **/
#define REG_BMC_INT			0x204
#define REG_BMC_BURST_TX_MODE		0x208
#define REG_BMC_PIO_MODE_READY  	0x20C

/** MISC register **/
#define REG_BMC_DATA_PORT		0x300
#define REG_MLC_INT_CONTROL		0x304
#define REG_PAGE_COUNT			0x308
#define REG_MLC_SW_RESET		0x30C
#define REG_REVISION			0x3F8
#define REG_CONFIGURATION		0x3FC


#define NANDC_MLC_ECC_EN            	BIT8
#define NANDC_NANDC_SW_RESET		BIT2
#define NANDC_BMC_SW_RESET		BIT1
#define NANDC_ECC_SW_RESET		BIT0

/* 0x10: ECC status register */
#define NANDC_ECC_CORR_FAIL		BIT3
#define NANDC_ECC_ERR_OCCUR		BIT2
#define NANDC_ECC_DEC_CP		BIT1
#define NANDC_ECC_ENC_CP		BIT0

/* 0x100: NAND flash host controller status register */
#define NANDC_BLANK_CHECK_FAIL		BIT7
#define NANDC_ECC_FAIL_TIMEOUT		BIT5
#define NANDC_STS_FAIL			BIT4
#define NANDC_LSN_CRC_FAIL		BIT3
#define NANDC_CMD_CP			BIT2
#define NANDC_BUSY			BIT1
#define NANDC_CHIP_ENABLE		BIT0

/* 0x104: Access control register */
#define NANDC_CMD_READ_ID		1
#define NANDC_CMD_RESET 		2
#define NANDC_CMD_READ_STS		4
#define NANDC_CMD_READ_EDC_STS		11
#define NANDC_CMD_PAGE_READ		5
#define NANDC_CMD_SPARE_READ		6
#define NANDC_CMD_BLANK_CHK		28
#define NANDC_CMD_PAGE_WRITE_RS 	16
#define NANDC_CMD_BLK_ERASE_RS		17
#define NANDC_CMD_COPY_BACK_RS		18
#define NANDC_CMD_SPARE_WRITE_RS	19
#define NANDC_CMD_2P_PAGE_WRITE_RS	24
#define NANDC_CMD_2P_BLK_ERASE_RS	25
#define NANDC_CMD_2P_COPY_BACK_RS	26
#ifdef FLASH_PAGE_4K
#define NANDC_CMD_SPARE_READALL 	14
#endif

#define NANDC_CMD_OFFSET		8

#define NANDC_CMD_LAUNCH		BIT7


/* 0x108: Flow control register */
#define NANDC_IO_WIDTH_16BIT		BIT4
#define NANDC_WP			BIT3
#define NANDC_PASS_STATUS_CHK_FAIL	BIT2
#define NANDC_MICRON_CMD		BIT1
#define NANDC_SKIP_ZERO_BLANK_CHK	BIT0
#define NANDC_IO_WIDTH_8BIT		0

/* 0x110: Memory module configuration register */
#define NANDC_BS_16P			0x00
#define NANDC_BS_32P			BIT16
#define NANDC_BS_64P			BIT17
#define NANDC_BS_128P			(BIT16|BIT17)

#define NANDC_MA_SINGLE 		0x0
#define NANDC_MA_TWO_PLANE		BIT14

#define NANDC_IT_NI			0x00
#define NANDC_IT_TWO			BIT12
#define NANDC_IT_FOUR			BIT13

#define NANDC_AP_3C			0x00
#define NANDC_AP_4C			BIT10
#define NANDC_AP_5C			BIT11

#define NANDC_PS_512			0x00
#define NANDC_PS_2K			BIT8
#define NANDC_PS_4K			BIT9

#define NANDC_MS_16MB			0x00
#define NANDC_MS_32MB			BIT4
#define NANDC_MS_64MB			BIT5
#define NANDC_MS_128MB			(BIT4|BIT5)
#define NANDC_MS_256MB			BIT6
#define NANDC_MS_512MB			(BIT4|BIT6)
#define NANDC_MS_1GB			(BIT5|BIT6)
#define NANDC_MS_2GB			(BIT4|BIT5|BIT6)
#define NANDC_MS_4GB			BIT7
#define NANDC_MS_8GB			(BIT7|BIT4)
#define NANDC_MS_16GB			(BIT7|BIT5)
#define NANDC_MS_32GB			(BIT7|BIT5|BIT4)

#define NANDC_MOD0_ENABLE		BIT0
#define NANDC_MOD1_ENABLE		BIT1
#define NANDC_MOD2_ENABLE		BIT2
#define NANDC_MOD3_ENABLE		BIT3

#endif	/* FTNANDC021_H */
