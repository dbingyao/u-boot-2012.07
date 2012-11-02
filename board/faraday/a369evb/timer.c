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

#include <common.h>
#include <div64.h>
#include <asm/io.h>
#include <asm/arch/ftpwmtmr010.h>

DECLARE_GLOBAL_DATA_PTR;

#define TIMER_LOAD_VAL	0xffffffff

static ulong timer_clock  = 66000000;  /* source clock (66MHz by default) */

static inline unsigned long long tick_to_time(unsigned long long tick)
{
	tick *= CONFIG_SYS_HZ;
	do_div(tick, gd->timer_rate_hz);

	return tick;
}

static inline unsigned long long usec_to_tick(unsigned long long usec)
{
	usec *= gd->timer_rate_hz;
	do_div(usec, 1000000);

	return usec;
}

int timer_init(void)
{
	struct ftpwmtmr010 *tmr = (struct ftpwmtmr010 *)CONFIG_FTPWMTMR010_BASE;

	debug("%s()\n", __func__);

	timer_clock = clk_get_rate("APB");

        /* timer reset */
	writel(0, &tmr->timer3_control);
	writel(0, &tmr->timer3_compare);

	/* set initial timer counter */
	writel(TIMER_LOAD_VAL, &tmr->timer3_count);

	writel((FTPWMTMR010_CTRL_START | FTPWMTMR010_CTRL_UPDATE) , &tmr->timer3_control);

	gd->timer_rate_hz = timer_clock;
	gd->tbu = gd->tbl = 0;

	return 0;
}

/*
 * Get the current 64 bit timer tick count
 */
unsigned long long get_ticks(void)
{
	struct ftpwmtmr010 *tmr = (struct ftpwmtmr010 *)CONFIG_FTPWMTMR010_BASE;
	ulong now = TIMER_LOAD_VAL - readl(&tmr->timer3_observe);

	/* increment tbu if tbl has rolled over */
	if (now < gd->tbl) {
		/* reload the timer counter value */
		writel((FTPWMTMR010_CTRL_START | FTPWMTMR010_CTRL_UPDATE) , &tmr->timer3_control);
		gd->tbu++;
	}
	gd->tbl = now;

	return (((unsigned long long)gd->tbu) << 32) | gd->tbl;
}

void __udelay(unsigned long usec)
{
	unsigned long long start;
	ulong tmo;

	start = get_ticks();		/* get current timestamp */
	tmo = usec_to_tick(usec);	/* convert usecs to ticks */
	while ((get_ticks() - start) < tmo)
		;			/* loop till time has passed */
}

/*
 * get_timer(base) can be used to check for timeouts or
 * to measure elasped time relative to an event:
 *
 * ulong start_time = get_timer(0) sets start_time to the current
 * time value.
 * get_timer(start_time) returns the time elapsed since then.
 *
 * The time is used in CONFIG_SYS_HZ units!
 */
ulong get_timer(ulong base)
{
	unsigned long long ticks, time;

	ticks = get_ticks();

	time = tick_to_time(ticks);

	return (time - base);
}

/*
 * Return the number of timer ticks per second.
 */
ulong get_tbclk(void)
{
	return gd->timer_rate_hz;
}
