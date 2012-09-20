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
#ifndef __ASM_ARCH_ARM_DMA_MAPPING_H
#define __ASM_ARCH_ARM_DMA_MAPPING_H

enum dma_data_direction {
	DMA_BIDIRECTIONAL	= 0,
	DMA_TO_DEVICE		= 1,
	DMA_FROM_DEVICE		= 2,
};

static void *dma_alloc_coherent(size_t len, unsigned long *handle)
{
	*handle = (unsigned long)memalign(64, len);

#ifndef CONFIG_SYS_DCACHE_OFF
{
	DECLARE_GLOBAL_DATA_PTR;
	u32 *page_table = (u32 *)gd->tlb_addr;
	u32 ca_start, ca_end;

	dcache_disable();
	asm volatile ("mov r3,#0\n"
		      "mcr p15, 0, r3, c8, c6, 0\n"  /* invalidate DTLB all */
		      : : : "r3");
	ca_start = (*handle >> 20);
	ca_end = (*handle >> 20) + (len & ~0xFFFFF);
	for ( ; ca_start <= ca_end ; ca_start ++) {

		page_table[ca_start] &= ~0xC;
	}
	dcache_enable();
}
#endif
	return (void *)*handle;
}

static inline unsigned long dma_map_single(volatile void *vaddr, size_t len,
					   enum dma_data_direction dir)
{
#ifndef CONFIG_SYS_DCACHE_OFF
	switch(dir) {
	case DMA_BIDIRECTIONAL:
	case DMA_TO_DEVICE:
		flush_dcache_range((unsigned long)vaddr, (unsigned long)vaddr + len);
		break;
	case DMA_FROM_DEVICE:
		invalidate_dcache_range((unsigned long)vaddr, (unsigned long)vaddr + len);
		break;
	}
#endif
	return (unsigned long)vaddr;
}

static inline void dma_unmap_single(volatile void *vaddr, size_t len,
				    unsigned long paddr)
{
}

#endif /* __ASM_ARM_DMA_MAPPING_H */
