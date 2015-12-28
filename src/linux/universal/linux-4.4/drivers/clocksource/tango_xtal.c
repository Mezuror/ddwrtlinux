#include <linux/clocksource.h>
#include <linux/sched_clock.h>
#include <linux/of_address.h>
#include <linux/printk.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/clk.h>

static void __iomem *xtal_in_cnt;
static struct delay_timer delay_timer;

static unsigned long notrace read_xtal_counter(void)
{
	return readl_relaxed(xtal_in_cnt);
}

static u64 notrace read_sched_clock(void)
{
	return read_xtal_counter();
}

static cycle_t read_clocksource(struct clocksource *cs)
{
	return read_xtal_counter();
}

static struct clocksource tango_xtal = {
	.name	= "tango-xtal",
	.rating	= 350,
	.read	= read_clocksource,
	.mask	= CLOCKSOURCE_MASK(32),
	.flags	= CLOCK_SOURCE_IS_CONTINUOUS,
};

static void __init tango_clocksource_init(struct device_node *np)
{
	struct clk *clk;
	int xtal_freq, ret;

	xtal_in_cnt = of_iomap(np, 0);
	if (xtal_in_cnt == NULL) {
		pr_err("%s: invalid address\n", np->full_name);
		return;
	}

	clk = of_clk_get(np, 0);
	if (IS_ERR(clk)) {
		pr_err("%s: invalid clock\n", np->full_name);
		return;
	}

	xtal_freq = clk_get_rate(clk);
	delay_timer.freq = xtal_freq;
	delay_timer.read_current_timer = read_xtal_counter;

	ret = clocksource_register_hz(&tango_xtal, xtal_freq);
	if (ret != 0) {
		pr_err("%s: registration failed\n", np->full_name);
		return;
	}

	sched_clock_register(read_sched_clock, 32, xtal_freq);
	register_current_timer_delay(&delay_timer);
}

CLOCKSOURCE_OF_DECLARE(tango, "sigma,tick-counter", tango_clocksource_init);
