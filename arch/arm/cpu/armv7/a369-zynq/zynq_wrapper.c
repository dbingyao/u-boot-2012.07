
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

#include <asm/io.h>
/* Common IO for xgmac and xnand */
/* Data Memory Barrier */
#define dmb() __asm__ __volatile__ ("dmb" : : : "memory")
#define SYNCHRONIZE_IO dmb()

void XIo_Out32(u32 OutAddress, u32 Value)
{
        *(volatile u32 *) OutAddress = Value;
        SYNCHRONIZE_IO;
}

u32 XIo_In32(u32 InAddress)
{
        volatile u32 temp = *(volatile u32 *)InAddress;
        SYNCHRONIZE_IO;
        return temp;
}       


