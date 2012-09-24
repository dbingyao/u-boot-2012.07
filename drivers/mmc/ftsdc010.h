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

#ifndef _FTSDC010_H
#define _FTSDC010_H

/* sd controller register */
#define REG_CMD                0x00000000
#define REG_ARG                0x00000004
#define REG_RSP0               0x00000008	/* response */
#define REG_RSP1               0x0000000C
#define REG_RSP2               0x00000010
#define REG_RSP3               0x00000014
#define REG_RSPCMD             0x00000018	/* responsed command */
#define REG_DCR                0x0000001C	/* data control */
#define REG_DTR                0x00000020	/* data timeout */
#define REG_DLR                0x00000024	/* data length */
#define REG_STR                0x00000028	/* status register */
#define REG_SCR                0x0000002C	/* status clear register */
#define REG_IMR                0x00000030	/* interrupt mask register */
#define REG_PWR                0x00000034	/* power control */
#define REG_CLK                0x00000038	/* clock control */
#define REG_BUS                0x0000003C	/* bus width */
#define REG_DWR                0x00000040	/* data window */
#define REG_GPOR               0x00000048
#define REG_FEA                0x0000009C
#define REG_REV                0x000000A0

/* bit mapping of command register */
#define CMD_IDX                0x0000003F
#define CMD_WAIT_RSP           0x00000040
#define CMD_LONG_RSP           0x00000080
#define CMD_APP                0x00000100
#define CMD_EN                 0x00000200
#define CMD_RST                0x00000400

/* bit mapping of response command register */
#define RSP_CMDIDX             0x0000003F
#define RSP_CMDAPP             0x00000040

/* bit mapping of data control register */
#define DCR_BKSZ               0x0000000F
#define DCR_WR                 0x00000010
#define DCR_RD                 0x00000000
#define DCR_DMA                0x00000020
#define DCR_EN                 0x00000040
#define DCR_THRES              0x00000080
#define DCR_BURST1             0x00000000
#define DCR_BURST4             0x00000100
#define DCR_BURST8             0x00000200
#define DCR_FIFO_RESET         0x00000400

/* bit mapping of status register */
#define STR_RSP_CRC            0x00000001
#define STR_DAT_CRC            0x00000002
#define STR_RSP_TIMEOUT        0x00000004
#define STR_DAT_TIMEOUT        0x00000008
#define STR_RSP_ERR            (STR_RSP_CRC | STR_RSP_TIMEOUT)
#define STR_DAT_ERR            (STR_DAT_CRC | STR_DAT_TIMEOUT)
#define STR_RSP                0x00000010
#define STR_DAT                0x00000020
#define STR_CMD                0x00000040
#define STR_DAT_END            0x00000080
#define STR_TXRDY              0x00000100
#define STR_RXRDY              0x00000200
#define STR_CARD_CHANGE        0x00000400
#define STR_CARD_REMOVED       0x00000800
#define STR_WPROT              0x00001000
#define STR_SDIO               0x00010000
#define STR_DAT0               0x00020000

/* bit mapping of clock register */
#define CLK_HISPD              0x00000200
#define CLK_OFF                0x00000100
#define CLK_SD                 0x00000080

/* bit mapping of bus register */
#define BUS_CARD_DATA3         0x00000020
#define BUS_4BITS_SUPP         0x00000008
#define BUS_8BITS_SUPP         0x00000010

#endif
