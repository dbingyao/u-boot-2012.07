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
#include <asm/arch/interrupts.h>

#define TIMER_LOAD_VAL 0xffffff

unsigned int ticks;
int init, cnt;

/**
 * Run 5 times only for simple test
 */
void timer_isr(void *data)
{
	/* Clear Interrupt */
	*((volatile int *)(0x92300000)) = 1;

	ticks ++;
	cnt ++;

	/* Reload Value */
	if (cnt < 5)
		*((volatile int *)(0x92300010)) = 0x26;
	else
		*((volatile int *)(0x92300010)) = 0x20;
}

/**
 * Timer(PWMTMR) use IRQ number 8~11 for Timer 0~3.
 * Level triggered, high active.
 */
static void timer0_init(void)
{
	printf("GIC test: Initialize Timer 0\n");

	irq_set_type(8, IRQ_TYPE_LEVEL_HIGH);
	irq_install_handler (8, timer_isr, NULL);
	irq_set_enable(8);

}

static int do_gic_test(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	const char *cmd;

	if (argc < 1)
		goto usage;

	cmd = argv[0];

	if (strcmp(cmd, "tirq") != 0)
		goto usage;

	if (!init) {
		enable_interrupts();

		timer0_init();

		init = 1;
	}

	cnt = 0;
	/* Timer Load Value */
	*((volatile int *)(0x92300014)) = TIMER_LOAD_VAL;
	/* Timer control */
	*((volatile int *)(0x92300010)) = 0x26;

	return CMD_RET_SUCCESS;

usage:
	return CMD_RET_USAGE;
}

static int do_print_ticks(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

	printf ("ticks value %d \n", ticks);

	return 0;
}

U_BOOT_CMD(
	tirq,	1,	1,	do_gic_test,
	"ARM Generic Interrupt Controller test",
	"tirq\n"
);

U_BOOT_CMD(
	pt,	1,	1,	do_print_ticks,
	"print GIC test ticks",
	"pt\n"
);
