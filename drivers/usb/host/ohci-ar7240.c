/*
 *  OHCI HCD (Host Controller Driver) for USB.
 *
 *  Bus Glue for Atheros AR71xx built-in OHCI controller.
 *
 *  Copyright (C) 2008 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  Parts of this file are based on Atheros' 2.6.15 BSP
 *	Copyright (C) 2007 Atheros Communications, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <asm/mach-ar7240/ar7240.h>
#include <linux/delay.h>

extern int usb_disabled(void);

static void 
ar7240_start_hc(struct platform_device *dev)
{
   // int mask = AR7240_RESET_USB_HOST|AR7240_RESET_USB_PHY;

	printk(KERN_DEBUG __FILE__
		": starting AR7240 OHCI USB Controller...\n");

    ar7240_reg_rmw_set(AR7240_RESET, AR7240_RESET_USB_OHCI_DLL);
    udelay(500);
    ar7240_reg_rmw_clear(AR7240_RESET, AR7240_RESET_USB_OHCI_DLL);
    ar7240_reg_rmw_set(AR7240_RESET, AR7240_RESET_USB_HOST);
    udelay(500);
    ar7240_reg_rmw_clear(AR7240_RESET, AR7240_RESET_USB_HOST);
    udelay(500);
    ar7240_reg_rmw_set(AR7240_RESET, AR7240_RESET_USB_OHCI_DLL);

    printk("%s val %x , reg:%x\n",__func__,ar7240_reg_rd(AR7240_RESET),AR7240_RESET_USB_OHCI_DLL);
    ar7240_reg_wr(AR7240_USB_FLADJ_VAL,0x3); 


}

static int usb_hcd_ar7240_probe(const struct hc_driver *driver,
				struct platform_device *pdev)
{
	struct usb_hcd *hcd;
	struct resource *res;
	int irq;
	int ret;

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res) {
		dev_dbg(&pdev->dev, "no IRQ specified\n");
		return -ENODEV;
	}
	irq = res->start;

	hcd = usb_create_hcd(driver, &pdev->dev,  pdev->dev.bus->name);
	if (!hcd)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_dbg(&pdev->dev, "no base address specified\n");
		ret = -ENODEV;
		goto err_put_hcd;
	}
	hcd->rsrc_start	= res->start;
	hcd->rsrc_len	= res->end - res->start + 1;

	if (!request_mem_region(hcd->rsrc_start, hcd->rsrc_len, hcd_name)) {
		dev_dbg(&pdev->dev, "controller already in use\n");
		ret = -EBUSY;
		goto err_put_hcd;
	}

	hcd->regs = ioremap(hcd->rsrc_start, hcd->rsrc_len);
	if (!hcd->regs) {
		dev_dbg(&pdev->dev, "error mapping memory\n");
		ret = -EFAULT;
		goto err_release_region;
	}
	ar7240_start_hc(pdev);

	ohci_hcd_init(hcd_to_ohci(hcd));

	ret = usb_add_hcd(hcd, irq, 0);
	if (ret)
		goto err_stop_hcd;

	return 0;

 err_stop_hcd:
	iounmap(hcd->regs);
 err_release_region:
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
 err_put_hcd:
	usb_put_hcd(hcd);
	return ret;
}

void usb_hcd_ar7240_remove(struct usb_hcd *hcd, struct platform_device *pdev)
{
	usb_remove_hcd(hcd);
	iounmap(hcd->regs);
	release_mem_region(hcd->rsrc_start, hcd->rsrc_len);
	usb_put_hcd(hcd);
}

static int ohci_ar7240_start(struct usb_hcd *hcd)
{
	struct ohci_hcd	*ohci = hcd_to_ohci(hcd);
	int ret;

	ret = ohci_init(ohci);
	if (ret < 0)
		return ret;

	ret = ohci_run(ohci);
	if (ret < 0)
		goto err;

	return 0;

 err:
	ohci_stop(hcd);
	return ret;
}

static const struct hc_driver ohci_ar7240_hc_driver = {
	.description		= hcd_name,
	.product_desc		= "Atheros AR7240 built-in OHCI controller",
	.hcd_priv_size		= sizeof(struct ohci_hcd),

	.irq			= ohci_irq,
	.flags			= HCD_USB11 | HCD_MEMORY,

	.start			= ohci_ar7240_start,
	.stop			= ohci_stop,
	.shutdown		= ohci_shutdown,

	.urb_enqueue		= ohci_urb_enqueue,
	.urb_dequeue		= ohci_urb_dequeue,
	.endpoint_disable	= ohci_endpoint_disable,

	/*
	 * scheduling support
	 */
	.get_frame_number	= ohci_get_frame,

	/*
	 * root hub support
	 */
	.hub_status_data	= ohci_hub_status_data,
	.hub_control		= ohci_hub_control,
	.start_port_reset	= ohci_start_port_reset,
};

static int ohci_hcd_ar7240_drv_probe(struct platform_device *pdev)
{
	if (usb_disabled())
		return -ENODEV;

	return usb_hcd_ar7240_probe(&ohci_ar7240_hc_driver, pdev);
}

static int ohci_hcd_ar7240_drv_remove(struct platform_device *pdev)
{
	struct usb_hcd *hcd = platform_get_drvdata(pdev);

	usb_hcd_ar7240_remove(hcd, pdev);
	return 0;
}

MODULE_ALIAS("platform:ar7240-ohci");

static struct platform_driver ohci_hcd_ar7240_driver = {
	.probe		= ohci_hcd_ar7240_drv_probe,
	.remove		= ohci_hcd_ar7240_drv_remove,
	.shutdown	= usb_hcd_platform_shutdown,
	.driver		= {
		.name	= "ar7240-ohci",
		.owner	= THIS_MODULE,
	},
};
