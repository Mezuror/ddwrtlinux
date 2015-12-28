/*
 * Driver for Epson RTC-7301SF/DG
 *
 * Copyright (C) 2009 Jose Vasconcellos
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/rtc.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/bcd.h>

#define RTC_NAME "rtc7301"
#define RTC_VERSION "0.1"

/* Epson RTC-7301 register addresses */
#define RTC7301_SEC		0x00
#define RTC7301_SEC10		0x01
#define RTC7301_MIN		0x02
#define RTC7301_MIN10		0x03
#define RTC7301_HOUR		0x04
#define RTC7301_HOUR10		0x05
#define RTC7301_WEEKDAY		0x06
#define RTC7301_DAY		0x07
#define RTC7301_DAY10		0x08
#define RTC7301_MON		0x09
#define RTC7301_MON10		0x0A
#define RTC7301_YEAR		0x0B
#define RTC7301_YEAR10		0x0C
#define RTC7301_YEAR100		0x0D
#define RTC7301_YEAR1000	0x0E
#define RTC7301_CTRLREG		0x0F

static uint8_t __iomem *rtc7301_base;

#define read_reg(offset) (readb(rtc7301_base + offset) & 0xf)
#define write_reg(offset, data) writeb(data, rtc7301_base + (offset))

#define rtc7301_isbusy() (read_reg(RTC7301_CTRLREG) & 1)

static void rtc7301_init_settings(void)
{
	int i;

	write_reg(RTC7301_CTRLREG, 2);
	write_reg(RTC7301_YEAR1000, 2);
	udelay(122);

	/* bank 1 */
	write_reg(RTC7301_CTRLREG, 6);
	for (i=0; i<15; i++)
		write_reg(i, 0);

	/* bank 2 */
	write_reg(RTC7301_CTRLREG, 14);
	for (i=0; i<15; i++)
		write_reg(i, 0);
	write_reg(RTC7301_CTRLREG, 0);
}

static int rtc7301_get_datetime(struct device *dev, struct rtc_time *dt)
{
	int cnt;
	uint8_t buf[16];

	cnt = 0;
	while (rtc7301_isbusy()) {
		udelay(244);
		if (cnt++ > 100) {
			dev_err(dev, "%s: timeout error %x\n", __func__, rtc7301_base[RTC7301_CTRLREG]);
			return -EIO;
		}
	}

	for (cnt=0; cnt<16; cnt++)
		buf[cnt] = read_reg(cnt);

	if (buf[RTC7301_SEC10] & 8) {
		dev_err(dev, "%s: RTC not set\n", __func__);
		return -EINVAL;
	}

	memset(dt, 0, sizeof(*dt));

	dt->tm_sec =  buf[RTC7301_SEC] + buf[RTC7301_SEC10]*10;
	dt->tm_min =  buf[RTC7301_MIN] + buf[RTC7301_MIN10]*10;
	dt->tm_hour = buf[RTC7301_HOUR] + buf[RTC7301_HOUR10]*10;

	dt->tm_mday = buf[RTC7301_DAY] + buf[RTC7301_DAY10]*10;
	dt->tm_mon =  buf[RTC7301_MON] + buf[RTC7301_MON10]*10 - 1;
	dt->tm_year = buf[RTC7301_YEAR] + buf[RTC7301_YEAR10]*10 +
		      buf[RTC7301_YEAR100]*100 +
		      ((buf[RTC7301_YEAR1000] & 3)*1000) - 1900;

	/* the rtc device may contain illegal values on power up
	 * according to the data sheet. make sure they are valid.
	 */

	return rtc_valid_tm(dt);
}

static int rtc7301_set_datetime(struct device *dev, struct rtc_time *dt)
{
	int data;

	data = dt->tm_year + 1900;
	if (data >= 2100 || data < 1900)
		return -EINVAL;

	write_reg(RTC7301_CTRLREG, 2);
       	udelay(122);

	data = bin2bcd(dt->tm_sec);
	write_reg(RTC7301_SEC, data);
	write_reg(RTC7301_SEC10, (data >> 4));

	data = bin2bcd(dt->tm_min);
	write_reg(RTC7301_MIN, data );
	write_reg(RTC7301_MIN10, (data >> 4));

	data = bin2bcd(dt->tm_hour);
	write_reg(RTC7301_HOUR, data);
	write_reg(RTC7301_HOUR10, (data >> 4));

	data = bin2bcd(dt->tm_mday);
	write_reg(RTC7301_DAY, data);
	write_reg(RTC7301_DAY10, (data>> 4));

	data = bin2bcd(dt->tm_mon + 1);
	write_reg(RTC7301_MON, data);
	write_reg(RTC7301_MON10, (data >> 4));

	data = bin2bcd(dt->tm_year % 100);
	write_reg(RTC7301_YEAR, data);
	write_reg(RTC7301_YEAR10, (data >> 4));
	data = bin2bcd((1900 + dt->tm_year) / 100);
	write_reg(RTC7301_YEAR100, data);

	data = bin2bcd(dt->tm_wday);
	write_reg(RTC7301_WEEKDAY, data);

	write_reg(RTC7301_CTRLREG, 0);

	return 0;
}

static const struct rtc_class_ops rtc7301_rtc_ops = {
	.read_time	= rtc7301_get_datetime,
	.set_time	= rtc7301_set_datetime,
};

static int rtc7301_probe(struct platform_device *pdev)
{
	struct rtc_device *rtc;
	struct resource *res;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENOENT;

	rtc7301_base = ioremap_nocache(res->start, 0x1000 /*res->end - res->start + 1*/);
	if (!rtc7301_base)
		return -EINVAL;

	rtc = rtc_device_register(RTC_NAME, &pdev->dev,
				&rtc7301_rtc_ops, THIS_MODULE);
	if (IS_ERR(rtc)) {
		iounmap(rtc7301_base);
		return PTR_ERR(rtc);
	}

	platform_set_drvdata(pdev, rtc);

	rtc7301_init_settings();
	return 0;
}

static int rtc7301_remove(struct platform_device *pdev)
{
	struct rtc_device *rtc = platform_get_drvdata(pdev);

	if (rtc)
		rtc_device_unregister(rtc);
	if (rtc7301_base)
		iounmap(rtc7301_base);
	return 0;
}

static struct platform_driver rtc7301_driver = {
	.driver = {
		.name	= RTC_NAME,
		.owner	= THIS_MODULE,
	},
	.probe	= rtc7301_probe,
	.remove = rtc7301_remove,
};

static __init int rtc7301_init(void)
{
	return platform_driver_register(&rtc7301_driver);
}
module_init(rtc7301_init);

static __exit void rtc7301_exit(void)
{
	platform_driver_unregister(&rtc7301_driver);
}
module_exit(rtc7301_exit);

MODULE_DESCRIPTION("Epson 7301 RTC driver");
MODULE_AUTHOR("Jose Vasconcellos <jvasco@verizon.net>");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:" RTC_NAME);
MODULE_VERSION(RTC_VERSION);
