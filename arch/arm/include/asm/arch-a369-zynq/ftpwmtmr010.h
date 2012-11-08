/*
 * (C) Copyright 2012 Faraday Technology
 * Bing-Yao, Luo <bjluo@faraday-tech.com>
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

/*
 * Timer
 */
#ifndef __FTPWMTMR010_H
#define __FTPWMTMR010_H

struct ftpwmtmr010{
        volatile uint32_t       interrupt_state;/* 0x00 */
        volatile uint32_t       reserved1;      /* 0x04 */
        volatile uint32_t       reserved2;      /* 0x08 */
        volatile uint32_t       reserved3;      /* 0x0C */
        volatile uint32_t       timer1_control; /* 0x10 */
        volatile uint32_t       timer1_count;   /* 0x14 */
        volatile uint32_t       timer1_compare; /* 0x18 */
        volatile uint32_t       timer1_observe; /* 0x1C RO */
        volatile uint32_t       timer2_control; /* 0x20 */
        volatile uint32_t       timer2_count;   /* 0x24 */
        volatile uint32_t       timer2_compare; /* 0x28 */
        volatile uint32_t       timer2_observe; /* 0x2C RO */
        volatile uint32_t       timer3_control; /* 0x30 */
        volatile uint32_t       timer3_count;   /* 0x34 */
        volatile uint32_t       timer3_compare; /* 0x38 */
        volatile uint32_t       timer3_observe; /* 0x3C RO */
        volatile uint32_t       timer4_control; /* 0x40 */
        volatile uint32_t       timer4_count;   /* 0x44 */
        volatile uint32_t       timer4_compare; /* 0x48 */
        volatile uint32_t       timer4_observe; /* 0x4C RO */
} ;


        /* for Timer Control Register */
#define FTPWMTMR010_CTRL_EXTCLK       (1 << 0)
#define FTPWMTMR010_CTRL_START        (1 << 1)
#define FTPWMTMR010_CTRL_UPDATE       (1 << 2)
#define FTPWMTMR010_CTRL_INVERTON     (1 << 3)
#define FTPWMTMR010_CTRL_AUTORELOAD   (1 << 4)
#define FTPWMTMR010_CTRL_INTRENABLE   (1 << 5)
#define FTPWMTMR010_CTRL_INTRPULSE    (1 << 6)
#define FTPWMTMR010_CTRL_DMAENABLE    (1 << 7)
#define FTPWMTMR010_CTRL_PWMENABLE    (1 << 8)
#define FTPWMTMR010_CTRL_DEADZONE     (0xFF << 24)

        /* for Timer Interrupt State & Mask Registers */
#define FTPWMTMR010_TM1_INTRSTS       (1 << 0)
#define FTPWMTMR010_TM2_INTRSTS       (1 << 1)
#define FTPWMTMR010_TM3_INTRSTS       (1 << 2)
#define FTPWMTMR010_TM4_INTRSTS       (1 << 3)
#define FTPWMTMR010_TM5_INTRSTS       (1 << 4)
#define FTPWMTMR010_TM6_INTRSTS       (1 << 5)
#define FTPWMTMR010_TM7_INTRSTS       (1 << 6)
#define FTPWMTMR010_TM8_INTRSTS       (1 << 7)

#endif	/* __FTPWMTMR010_H */
