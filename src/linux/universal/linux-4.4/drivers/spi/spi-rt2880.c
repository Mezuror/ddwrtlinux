/*
 * spi-rt2880.c -- Ralink RT288x/RT305x SPI controller driver
 *
 * Copyright (C) 2011 Sergiy <piratfm@gmail.com>
 * Copyright (C) 2011-2013 Gabor Juhos <juhosg@openwrt.org>
 *
 * Some parts are based on spi-orion.c:
 *   Author: Shadi Ammouri <shadi@marvell.com>
 *   Copyright (C) 2007-2008 Marvell Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/reset.h>
#include <linux/spi/spi.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>

#include <ralink_regs.h>

#define SPI_BPW_MASK(bits) BIT((bits) - 1)

#define DRIVER_NAME			"spi-rt2880"
/* in usec */
#define RALINK_SPI_WAIT_MAX_LOOP	2000

#define RAMIPS_SPI_DEV_OFFSET		0x40

#define RAMIPS_SPI_STAT(cs)		(0x00 + (cs * RAMIPS_SPI_DEV_OFFSET))
#define RAMIPS_SPI_CFG(cs)		(0x10 + (cs * RAMIPS_SPI_DEV_OFFSET))
#define RAMIPS_SPI_CTL(cs)		(0x14 + (cs * RAMIPS_SPI_DEV_OFFSET))
#define RAMIPS_SPI_DATA(cs)		(0x20 + (cs * RAMIPS_SPI_DEV_OFFSET))
#define RAMIPS_SPI_FIFO_STAT(cs)	(0x38 + (cs * RAMIPS_SPI_DEV_OFFSET))
#define RAMIPS_SPI_ARBITER		0xF0

/* SPISTAT register bit field */
#define SPISTAT_BUSY			BIT(0)

/* SPICFG register bit field */
#define SPICFG_LSBFIRST			0
#define SPICFG_MSBFIRST			BIT(8)
#define SPICFG_SPICLKPOL		BIT(6)
#define SPICFG_RXCLKEDGE_FALLING	BIT(5)
#define SPICFG_TXCLKEDGE_FALLING	BIT(4)
#define SPICFG_SPICLK_PRESCALE_MASK	0x7
#define SPICFG_SPICLK_DIV2		0
#define SPICFG_SPICLK_DIV4		1
#define SPICFG_SPICLK_DIV8		2
#define SPICFG_SPICLK_DIV16		3
#define SPICFG_SPICLK_DIV32		4
#define SPICFG_SPICLK_DIV64		5
#define SPICFG_SPICLK_DIV128		6
#define SPICFG_SPICLK_DISABLE		7

/* SPICTL register bit field */
#define SPICTL_HIZSDO			BIT(3)
#define SPICTL_STARTWR			BIT(2)
#define SPICTL_STARTRD			BIT(1)
#define SPICTL_SPIENA			BIT(0)

/* SPIFIFOSTAT register bit field */
#define SPIFIFOSTAT_TXFULL		BIT(17)

#define SPICTL_ARB_EN			BIT(31)
#define SPI1_POR			BIT(1)
#define SPI0_POR			BIT(0)

#define RT2880_SPI_MODE_BITS	(SPI_CPOL | SPI_CPHA | SPI_LSB_FIRST | SPI_CS_HIGH)

struct rt2880_spi;

struct rt2880_spi_ops {
	void (*init_hw)(struct rt2880_spi *rs);
	int num_cs;
};

struct rt2880_spi {
	struct spi_master	*master;
	void __iomem		*base;
	unsigned int		sys_freq;
	unsigned int		speed;
	struct clk		*clk;
	spinlock_t		lock;

	struct rt2880_spi_ops	*ops;
};

static inline struct rt2880_spi *spidev_to_rt2880_spi(struct spi_device *spi)
{
	return spi_master_get_devdata(spi->master);
}

static inline u32 rt2880_spi_read(struct rt2880_spi *rs, u32 reg)
{
	return ioread32(rs->base + reg);
}

static inline void rt2880_spi_write(struct rt2880_spi *rs, u32 reg, u32 val)
{
	iowrite32(val, rs->base + reg);
}

static inline void rt2880_spi_setbits(struct rt2880_spi *rs, u32 reg, u32 mask)
{
	void __iomem *addr = rs->base + reg;
	unsigned long flags;
	u32 val;

	spin_lock_irqsave(&rs->lock, flags);
	val = ioread32(addr);
	val |= mask;
	iowrite32(val, addr);
	spin_unlock_irqrestore(&rs->lock, flags);
}

static inline void rt2880_spi_clrbits(struct rt2880_spi *rs, u32 reg, u32 mask)
{
	void __iomem *addr = rs->base + reg;
	unsigned long flags;
	u32 val;

	spin_lock_irqsave(&rs->lock, flags);
	val = ioread32(addr);
	val &= ~mask;
	iowrite32(val, addr);
	spin_unlock_irqrestore(&rs->lock, flags);
}

static int rt2880_spi_baudrate_set(struct spi_device *spi, unsigned int speed)
{
	int cs = spi->chip_select;
	struct rt2880_spi *rs = spidev_to_rt2880_spi(spi);
	u32 rate;
	u32 prescale;
	u32 reg;

	dev_dbg(&spi->dev, "speed:%u\n", speed);

	/*
	 * the supported rates are: 2, 4, 8, ... 128
	 * round up as we look for equal or less speed
	 */
	rate = DIV_ROUND_UP(rs->sys_freq, speed);
	dev_dbg(&spi->dev, "rate-1:%u\n", rate);
	rate = roundup_pow_of_two(rate);
	dev_dbg(&spi->dev, "rate-2:%u\n", rate);

	/* check if requested speed is too small */
	if (rate > 128)
		return -EINVAL;

	if (rate < 2)
		rate = 2;

	/* Convert the rate to SPI clock divisor value.	*/
	prescale = ilog2(rate / 2);
	dev_dbg(&spi->dev, "prescale:%u\n", prescale);

	reg = rt2880_spi_read(rs, RAMIPS_SPI_CFG(cs));
	reg = ((reg & ~SPICFG_SPICLK_PRESCALE_MASK) | prescale);
	rt2880_spi_write(rs, RAMIPS_SPI_CFG(cs), reg);
	rs->speed = speed;
	return 0;
}

/*
 * called only when no transfer is active on the bus
 */
static int
rt2880_spi_setup_transfer(struct spi_device *spi, struct spi_transfer *t)
{
	struct rt2880_spi *rs = spidev_to_rt2880_spi(spi);
	unsigned int speed = spi->max_speed_hz;
	int rc, cs = spi->chip_select;
	u32 reg;

	if ((t != NULL) && t->speed_hz)
		speed = t->speed_hz;

	if (rs->speed != speed) {
		dev_dbg(&spi->dev, "speed_hz:%u\n", speed);
		rc = rt2880_spi_baudrate_set(spi, speed);
		if (rc)
			return rc;
	}

	reg = rt2880_spi_read(rs, RAMIPS_SPI_CFG(cs));

	reg = (reg & ~SPICFG_MSBFIRST);
	if (!(spi->mode & SPI_LSB_FIRST))
		reg |= SPICFG_MSBFIRST;

	reg = (reg & ~(SPICFG_SPICLKPOL | SPICFG_RXCLKEDGE_FALLING |SPICFG_TXCLKEDGE_FALLING));
	switch(spi->mode & (SPI_CPOL | SPI_CPHA)) {
		case SPI_MODE_0:
			reg |= SPICFG_SPICLKPOL | SPICFG_TXCLKEDGE_FALLING;
			break;
		case SPI_MODE_1:
			reg |= SPICFG_SPICLKPOL | SPICFG_RXCLKEDGE_FALLING;
			break;
		case SPI_MODE_2:
			reg |= SPICFG_RXCLKEDGE_FALLING;
			break;
		case SPI_MODE_3:
			reg |= SPICFG_TXCLKEDGE_FALLING;
			break;
	}

	rt2880_spi_write(rs, RAMIPS_SPI_CFG(cs), reg);

	reg = SPICTL_ARB_EN;
	if (spi->mode & SPI_CS_HIGH) {
		switch(cs) {
			case 0:
				reg |= SPI0_POR;
				break;
			case 1:
				reg |= SPI1_POR;
				break;
		}
	}

	rt2880_spi_write(rs, RAMIPS_SPI_ARBITER, reg);

	return 0;
}

static void rt2880_spi_set_cs(struct spi_device *spi, int enable)
{
	struct rt2880_spi *rs = spidev_to_rt2880_spi(spi);
	int cs = spi->chip_select;

	if (enable)
		rt2880_spi_clrbits(rs, RAMIPS_SPI_CTL(cs), SPICTL_SPIENA);
	else
		rt2880_spi_setbits(rs, RAMIPS_SPI_CTL(cs), SPICTL_SPIENA);
}

static inline int rt2880_spi_wait_till_ready(struct spi_device *spi)
{
	struct rt2880_spi *rs = spidev_to_rt2880_spi(spi);
	int cs = spi->chip_select;
	int i;

	for (i = 0; i < RALINK_SPI_WAIT_MAX_LOOP; i++) {
		u32 status;

		status = rt2880_spi_read(rs, RAMIPS_SPI_STAT(cs));
		if ((status & SPISTAT_BUSY) == 0)
			return 0;

		cpu_relax();
		udelay(1);
	}

	return -ETIMEDOUT;
}

static unsigned int
rt2880_spi_write_read(struct spi_device *spi, struct list_head *list, struct spi_transfer *xfer)
{
	struct rt2880_spi *rs = spidev_to_rt2880_spi(spi);
	int cs = spi->chip_select;
	unsigned count = 0;
	u8 *rx = xfer->rx_buf;
	const u8 *tx = xfer->tx_buf;
	int err;

	dev_dbg(&spi->dev, "read (%d): %s %s\n", xfer->len,
		  (tx != NULL) ? "tx" : "  ",
		  (rx != NULL) ? "rx" : "  ");

	if (tx) {
		for (count = 0; count < xfer->len; count++) {
			rt2880_spi_write(rs, RAMIPS_SPI_DATA(cs), tx[count]);
			rt2880_spi_setbits(rs, RAMIPS_SPI_CTL(cs), SPICTL_STARTWR);
			err = rt2880_spi_wait_till_ready(spi);
			if (err) {
				dev_err(&spi->dev, "TX failed, err=%d\n", err);
				goto out;
			}
		}
	}

	if (rx) {
		for (count = 0; count < xfer->len; count++) {
			rt2880_spi_setbits(rs, RAMIPS_SPI_CTL(cs), SPICTL_STARTRD);
			err = rt2880_spi_wait_till_ready(spi);
			if (err) {
				dev_err(&spi->dev, "RX failed, err=%d\n", err);
				goto out;
			}
			rx[count] = (u8) rt2880_spi_read(rs, RAMIPS_SPI_DATA(cs));
		}
	}

out:
	return count;
}

static int rt2880_spi_transfer_one_message(struct spi_master *master,
					   struct spi_message *m)
{
	struct rt2880_spi *rs = spi_master_get_devdata(master);
	struct spi_device *spi = m->spi;
	struct spi_transfer *t = NULL;
	int par_override = 0;
	int status = 0;
	int cs_active = 0;

	/* Load defaults */
	status = rt2880_spi_setup_transfer(spi, NULL);
	if (status < 0)
		goto msg_done;

	list_for_each_entry(t, &m->transfers, transfer_list) {
		if (t->tx_buf == NULL && t->rx_buf == NULL && t->len) {
			dev_err(&spi->dev,
				"message rejected: invalid transfer data buffers\n");
			status = -EIO;
			goto msg_done;
		}

		if (t->speed_hz && t->speed_hz < (rs->sys_freq / 128)) {
			dev_err(&spi->dev,
				"message rejected: device min speed (%d Hz) exceeds required transfer speed (%d Hz)\n",
				(rs->sys_freq / 128), t->speed_hz);
			status = -EIO;
			goto msg_done;
		}

		if (par_override || t->speed_hz || t->bits_per_word) {
			par_override = 1;
			status = rt2880_spi_setup_transfer(spi, t);
			if (status < 0)
				goto msg_done;
			if (!t->speed_hz && !t->bits_per_word)
				par_override = 0;
		}

		if (!cs_active) {
			rt2880_spi_set_cs(spi, 1);
			cs_active = 1;
		}

		if (t->len)
			m->actual_length += rt2880_spi_write_read(spi, &m->transfers, t);

		if (t->delay_usecs)
			udelay(t->delay_usecs);

		if (t->cs_change) {
			rt2880_spi_set_cs(spi, 0);
			cs_active = 0;
		}
	}

msg_done:
	if (cs_active)
		rt2880_spi_set_cs(spi, 0);

	m->status = status;
	spi_finalize_current_message(master);

	return 0;
}

static int rt2880_spi_setup(struct spi_device *spi)
{
	struct rt2880_spi *rs = spidev_to_rt2880_spi(spi);

	if ((spi->max_speed_hz == 0) ||
		(spi->max_speed_hz > (rs->sys_freq / 2)))
		spi->max_speed_hz = (rs->sys_freq / 2);

	if (spi->max_speed_hz < (rs->sys_freq / 128)) {
		dev_err(&spi->dev, "setup: requested speed is too low %d Hz\n",
			spi->max_speed_hz);
		return -EINVAL;
	}

	/*
	 * baudrate & width will be set rt2880_spi_setup_transfer
	 */
	return 0;
}

static void rt2880_spi_reset(struct rt2880_spi *rs)
{
	rt2880_spi_write(rs, RAMIPS_SPI_CFG(0),
			 SPICFG_MSBFIRST | SPICFG_TXCLKEDGE_FALLING |
			 SPICFG_SPICLK_DIV16 | SPICFG_SPICLKPOL);
	rt2880_spi_write(rs, RAMIPS_SPI_CTL(0), SPICTL_HIZSDO | SPICTL_SPIENA);
}

static void rt5350_spi_reset(struct rt2880_spi *rs)
{
	int cs;

	rt2880_spi_write(rs, RAMIPS_SPI_ARBITER,
			 SPICTL_ARB_EN);

	for (cs = 0; cs < rs->ops->num_cs; cs++) {
		rt2880_spi_write(rs, RAMIPS_SPI_CFG(cs),
				 SPICFG_MSBFIRST | SPICFG_TXCLKEDGE_FALLING |
				 SPICFG_SPICLK_DIV16 | SPICFG_SPICLKPOL);
		rt2880_spi_write(rs, RAMIPS_SPI_CTL(cs), SPICTL_HIZSDO | SPICTL_SPIENA);
	}
}

static struct rt2880_spi_ops spi_ops[] = {
	{
		.init_hw = rt2880_spi_reset,
		.num_cs = 1,
	}, {
		.init_hw = rt5350_spi_reset,
		.num_cs = 2,
	},
};

static const struct of_device_id rt2880_spi_match[] = {
	{ .compatible = "ralink,rt2880-spi", .data = &spi_ops[0]},
	{ .compatible = "ralink,rt5350-spi", .data = &spi_ops[1]},
	{},
};
MODULE_DEVICE_TABLE(of, rt2880_spi_match);

static int rt2880_spi_probe(struct platform_device *pdev)
{
	const struct of_device_id *match;
	struct spi_master *master;
	struct rt2880_spi *rs;
	unsigned long flags;
	void __iomem *base;
	struct resource *r;
	int status = 0;
	struct clk *clk;
	struct rt2880_spi_ops *ops;

	match = of_match_device(rt2880_spi_match, &pdev->dev);
	if (!match)
		return -EINVAL;
	ops = (struct rt2880_spi_ops *)match->data;

	r = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	base = devm_ioremap_resource(&pdev->dev, r);
	if (IS_ERR(base))
		return PTR_ERR(base);

	clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(clk)) {
		dev_err(&pdev->dev, "unable to get SYS clock, err=%d\n",
			status);
		return PTR_ERR(clk);
	}

	status = clk_prepare_enable(clk);
	if (status)
		return status;

	master = spi_alloc_master(&pdev->dev, sizeof(*rs));
	if (master == NULL) {
		dev_dbg(&pdev->dev, "master allocation failed\n");
		return -ENOMEM;
	}

	master->mode_bits = RT2880_SPI_MODE_BITS;

	master->setup = rt2880_spi_setup;
	master->transfer_one_message = rt2880_spi_transfer_one_message;
	master->bits_per_word_mask = SPI_BPW_MASK(8);
	master->dev.of_node = pdev->dev.of_node;
	master->num_chipselect = ops->num_cs;

	dev_set_drvdata(&pdev->dev, master);

	rs = spi_master_get_devdata(master);
	rs->base = base;
	rs->clk = clk;
	rs->master = master;
	rs->sys_freq = clk_get_rate(rs->clk);
	rs->ops = ops;
	dev_dbg(&pdev->dev, "sys_freq: %u\n", rs->sys_freq);
	spin_lock_irqsave(&rs->lock, flags);

	device_reset(&pdev->dev);

	rs->ops->init_hw(rs);

	return spi_register_master(master);
}

static int rt2880_spi_remove(struct platform_device *pdev)
{
	struct spi_master *master;
	struct rt2880_spi *rs;

	master = dev_get_drvdata(&pdev->dev);
	rs = spi_master_get_devdata(master);

	clk_disable(rs->clk);
	spi_unregister_master(master);

	return 0;
}

MODULE_ALIAS("platform:" DRIVER_NAME);

static struct platform_driver rt2880_spi_driver = {
	.driver = {
		.name = DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = rt2880_spi_match,
	},
	.probe = rt2880_spi_probe,
	.remove = rt2880_spi_remove,
};

module_platform_driver(rt2880_spi_driver);

MODULE_DESCRIPTION("Ralink SPI driver");
MODULE_AUTHOR("Sergiy <piratfm@gmail.com>");
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_LICENSE("GPL");
