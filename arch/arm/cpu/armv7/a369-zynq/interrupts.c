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


#include <common.h>
#include <linux/bitops.h>
#include <asm/arch/a369.h>
#include <asm/arch/interrupts.h>
#include <asm/proc-armv/ptrace.h>
#include <asm/arch/xparameters_ps.h>

#ifdef CONFIG_USE_IRQ

struct _irq_handler {
	void                *m_data;
	void (*m_func)( void *data);
};

static struct _irq_handler IRQ_HANDLER[NR_IRQS];

#ifdef CONFIG_SOC_ZYNQ

#define GIC_CPU_REG32(off)	*(volatile unsigned long *)(GIC_CPU_BASE + off)
#define GIC_DIST_REG32(off)	*(volatile unsigned long *)(GIC_DIST_BASE + off)

static void gic_dist_init(unsigned int gic_irqs)
{
	unsigned int i;

	GIC_DIST_REG32(GIC_DIST_CTRL) = 0;

	/*
	 * Set all global interrupts to be level triggered, active high.
	 */
	for (i = 32; i < gic_irqs; i += 16)
		GIC_DIST_REG32(GIC_DIST_CONFIG + i * 4 / 16) = 0x55555555;

	for (i = 32; i < gic_irqs; i += 4) {
		/*
		 * Set all global interrupts target to CPU 0 only.
		 */
		GIC_DIST_REG32(GIC_DIST_TARGET + i * 4 / 4) = 0x01010101;

		/*
		 * Set priority on all global interrupts.
		 */
		GIC_DIST_REG32(GIC_DIST_PRI + i * 4 / 4) = 0x0a0a0a0a;
	}

	/*
	 * Enable all interrupts.
	 */
	for (i = 32; i < gic_irqs; i += 32)
		GIC_DIST_REG32(GIC_DIST_ENABLE_CLEAR + i * 4 / 32) = 0;

	for (i = 32; i < gic_irqs; i += 32)
		GIC_DIST_REG32(GIC_DIST_ENABLE_SET + i * 4 / 32) = 0xffffffff;


	GIC_DIST_REG32(GIC_DIST_CTRL) = 1;
}

static void gic_cpu_init(void)
{
	int i;

	/*
	 * Do not deal with the banked PPI and SGI interrupts -
	 * disable all SGI and PPI interrupts.
	 */
	GIC_DIST_REG32(GIC_DIST_ENABLE_CLEAR) = 0xffffffff;
	GIC_DIST_REG32(GIC_DIST_ENABLE_SET) = 0x00000000;

	/*
	 * Set priority on PPI and SGI interrupts
	 */
	for (i = 0; i < 32; i += 4)
		GIC_DIST_REG32(GIC_DIST_PRI + i * 4 / 4) = 0x0a0a0a0a;

	GIC_CPU_REG32(GIC_CPU_PRIMASK) = 0xf0;
	GIC_CPU_REG32(GIC_CPU_CTRL) = 0;
}

/**
 * VINTC0 at A369 connect to IRQ 90 of GIC at ZYNQ board
 */
static void gic_init(void)
{
	unsigned int gic_irqs;

	gic_irqs = GIC_DIST_REG32(GIC_DIST_CTR) & 0x1f;
	gic_irqs = (gic_irqs + 1) * 32;
	if (gic_irqs > 1020)
		gic_irqs = 1020;
	printf("GIC: provides %d interrupts, %d external interrupt lines\n",
		gic_irqs, (gic_irqs - 32));

	gic_dist_init(gic_irqs);
	gic_cpu_init();
}

#endif

static void default_isr(void *data)
{
	printf("default_isr():  called for IRQ %d, VINTC IS=%x\n",
	       (int)data, *((volatile int *)(CONFIG_FTINTC0_BASE)));

}

void do_irq (struct pt_regs *pt_regs)
{
	int irq, vintc_is;

	/* Prevent Nested interrupts */
	disable_interrupts();

#ifdef CONFIG_SOC_ZYNQ
	/* Do not handle SGI and PPI */
	/* VINTC connect to IRQ 90 at GIC */
	int gic_is = GIC_DIST_REG32(GIC_DIST_PENDING_SET + 8);

	if (gic_is & (1 << 26)) {
#endif
		vintc_is = *((volatile int *) CONFIG_FTINTC0_BASE);

		irq = 0;
		while (irq < NR_IRQS) {
			if (vintc_is & (1 << irq))
				IRQ_HANDLER[irq].m_func(IRQ_HANDLER[irq].m_data);

			irq ++;

		}
#ifdef CONFIG_SOC_ZYNQ
	}
#endif

	enable_interrupts();
}

void irq_set_enable(int irq)
{
	int enable;

	enable = *((volatile int *)(CONFIG_FTINTC0_BASE + 0x4));
	enable |= (1 << irq);

	*((volatile int *)(CONFIG_FTINTC0_BASE + 0x4)) = enable;
}

void irq_set_disable(int irq)
{
	int enable;

	enable = *((volatile int *)(CONFIG_FTINTC0_BASE + 0x4));
	enable &= ~(1 << irq);

	*((volatile int *)(CONFIG_FTINTC0_BASE + 0x4)) = enable;
}

int irq_set_type(int irq, unsigned int type)
{
        int mode, level;

	if (irq >= NR_IRQS)
		return -1;

	/* Trigger Mode: 0 - Level, 1 - Edge */
	mode = *((volatile int *)(CONFIG_FTINTC0_BASE + 0xC));

	/* Trigger level: 0 - Active High, 1 - Active Low or Falling Edge */
	level = *((volatile int *)(CONFIG_FTINTC0_BASE + 0x10));

	switch (type) {
	case IRQ_TYPE_LEVEL_LOW:
		level |= (1 << irq);
		/* fall through */

	case IRQ_TYPE_LEVEL_HIGH:
		break;

	case IRQ_TYPE_EDGE_FALLING:
		level |= (1 << irq);
		/* fall through */

	case IRQ_TYPE_EDGE_RISING:
		mode |= (1 << irq);
		break;

	default:
		return -1;
	}

	*((volatile int *)(CONFIG_FTINTC0_BASE + 0xC)) = mode;

	*((volatile int *)(CONFIG_FTINTC0_BASE + 0x10)) = level;

        return 0;
}

void irq_install_handler (int irq, interrupt_handler_t handle_irq, void *data)
{
	if (irq >= NR_IRQS || !handle_irq)
		return;

	IRQ_HANDLER[irq].m_data = data;
	IRQ_HANDLER[irq].m_func = handle_irq;
}


int arch_interrupt_init (void)
{
	int i;

	/* install default interrupt handlers */
	for (i = 0; i < NR_IRQS; i++)
		irq_install_handler(i, default_isr, (void *)i);

	/* configure interrupts for IRQ mode */
	*((volatile int *)(CONFIG_FTINTC0_BASE + 0x4)) = 0;
	/* Interrupt Clear */
	*((volatile int *)(CONFIG_FTINTC0_BASE + 0x8)) = ~0;
	/* Trigger Mode: 0 - Level, 1 - Edge */
	*((volatile int *)(CONFIG_FTINTC0_BASE + 0xC)) = 0;
	/* Trigger level: 0 - Active High, 1 - Active Low */
	*((volatile int *)(CONFIG_FTINTC0_BASE + 0x10)) = 0;

#ifdef CONFIG_SOC_ZYNQ
	*((volatile int *)(CONFIG_SCU_BASE + 0x200)) = 0x5878;
	*((volatile int *)(CONFIG_SCU_BASE + 0x238)) = 0x200;
	*((volatile int *)(CONFIG_SCU_BASE + 0x23C)) = 0;

	gic_init();
#endif
	return (0);
}

int do_irqinfo (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int i;

	puts ("\nInterrupt-Information:\n\n"
	      "Nr  Routine   Arg       Count\n"
	      "-----------------------------\n");

	for (i = 0; i < NR_IRQS; i++) {
		if (IRQ_HANDLER[i].m_func != (interrupt_handler_t*) default_isr) {
			printf ("%02d  %08x  %08x \n", i,
				(int)IRQ_HANDLER[i].m_func, (int)IRQ_HANDLER[i].m_data);
		}
	}
	puts ("\n");
	return (0);
}
#endif
