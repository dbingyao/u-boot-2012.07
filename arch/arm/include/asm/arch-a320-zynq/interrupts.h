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

#ifdef CONFIG_USE_IRQ
enum {
	IRQ_TYPE_NONE           = 0x00000000,
	IRQ_TYPE_EDGE_RISING    = 0x00000001,
	IRQ_TYPE_EDGE_FALLING   = 0x00000002,
	IRQ_TYPE_EDGE_BOTH      = (IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING),
	IRQ_TYPE_LEVEL_HIGH     = 0x00000004,
	IRQ_TYPE_LEVEL_LOW      = 0x00000008,
	IRQ_TYPE_LEVEL_MASK     = (IRQ_TYPE_LEVEL_LOW | IRQ_TYPE_LEVEL_HIGH),
	IRQ_TYPE_SENSE_MASK     = 0x0000000f,
};

int irq_set_type(int irq, unsigned int type);
void irq_install_handler (int irq, interrupt_handler_t handle_irq, void *data);
void irq_set_enable(int irq);
void irq_set_disable(int irq);

#endif
