/*
 *  Ralink MT7620A SoC PCI support
 *
 *  Copyright (C) 2007-2013 Bruce Chang
 *  Copyright (C) 2013 John Crispin <blogic@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/types.h>
#include <linux/pci.h>
#include <linux/io.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_pci.h>
#include <linux/reset.h>
#include <linux/platform_device.h>

#include <asm/mach-ralink-openwrt/ralink_regs.h>
#include <asm/mach-ralink-openwrt/mt7620.h>

#define RALINK_PCI_MM_MAP_BASE		0x20000000
#define RALINK_PCI_IO_MAP_BASE		0x10160000

#define RALINK_INT_PCIE0		4
#define RALINK_SYSCFG1			0x14
#define RALINK_CLKCFG1			0x30
#define RALINK_GPIOMODE			0x60
#define RALINK_PCIE_CLK_GEN		0x7c
#define RALINK_PCIE_CLK_GEN1		0x80
#define PCIEPHY0_CFG			0x90
#define PPLL_CFG1			0x9c
#define PPLL_DRV			0xa0
#define PDRV_SW_SET			(1<<31)
#define LC_CKDRVPD_			(1<<19)

#define RALINK_PCI_CONFIG_ADDR		0x20
#define RALINK_PCI_CONFIG_DATA_VIRT_REG	0x24
#define MEMORY_BASE			0x0
#define RALINK_PCIE0_RST		(1<<26)
#define RALINK_PCI_BASE			0xB0140000
#define RALINK_PCI_MEMBASE		0x28
#define RALINK_PCI_IOBASE		0x2C

#define RT6855_PCIE0_OFFSET		0x2000

#define RALINK_PCI_PCICFG_ADDR		0x00
#define RALINK_PCI0_BAR0SETUP_ADDR	0x10
#define RALINK_PCI0_IMBASEBAR0_ADDR	0x18
#define RALINK_PCI0_ID			0x30
#define RALINK_PCI0_CLASS		0x34
#define RALINK_PCI0_SUBID		0x38
#define RALINK_PCI0_STATUS		0x50
#define RALINK_PCI_PCIMSK_ADDR		0x0C

#define RALINK_PCIEPHY_P0_CTL_OFFSET	0x7498
#define RALINK_PCIE0_CLK_EN		(1 << 26)

#define BUSY				0x80000000
#define WAITRETRY_MAX			10
#define WRITE_MODE			(1UL << 23)
#define DATA_SHIFT			0
#define ADDR_SHIFT			8

static void __iomem *bridge_base;
static void __iomem *pcie_base;

static struct reset_control *rstpcie0;

static inline void bridge_w32(u32 val, unsigned reg)
{
	iowrite32(val, bridge_base + reg);
}

static inline u32 bridge_r32(unsigned reg)
{
	return ioread32(bridge_base + reg);
}

static inline void pcie_w32(u32 val, unsigned reg)
{
	iowrite32(val, pcie_base + reg);
}

static inline u32 pcie_r32(unsigned reg)
{
	return ioread32(pcie_base + reg);
}

static inline void pcie_m32(u32 clr, u32 set, unsigned reg)
{
	u32 val = pcie_r32(reg);

	val &= ~clr;
	val |= set;
	pcie_w32(val, reg);
}

static int wait_pciephy_busy(void)
{
	unsigned long reg_value = 0x0, retry = 0;

	while (1) {
		reg_value = pcie_r32(PCIEPHY0_CFG);

		if (reg_value & BUSY)
			mdelay(100);
		else
			break;
		if (retry++ > WAITRETRY_MAX){
			printk("PCIE-PHY retry failed.\n");
			return -1;
		}
	}
	return 0;
}

static void pcie_phy(unsigned long addr, unsigned long val)
{
	wait_pciephy_busy();
	pcie_w32(WRITE_MODE | (val << DATA_SHIFT) | (addr << ADDR_SHIFT), PCIEPHY0_CFG);
	mdelay(1);
	wait_pciephy_busy();
}

static int pci_config_read(struct pci_bus *bus, unsigned int devfn, int where, int size, u32 * val)
{
	unsigned int slot = PCI_SLOT(devfn);
	u8 func = PCI_FUNC(devfn);
	u32 address;
	u32 data;
	u32 num = 0;

	if (bus)
		num = bus->number;

	address = (((where & 0xF00) >> 8) << 24) | (num << 16) | (slot << 11) | (func << 8) | (where & 0xfc) | 0x80000000;
	bridge_w32(address, RALINK_PCI_CONFIG_ADDR);
	data = bridge_r32(RALINK_PCI_CONFIG_DATA_VIRT_REG);

	switch (size) {
	case 1:
		*val = (data >> ((where & 3) << 3)) & 0xff;
		break;
	case 2:
		*val = (data >> ((where & 3) << 3)) & 0xffff;
		break;
	case 4:
		*val = data;
		break;
	}

	return PCIBIOS_SUCCESSFUL;
}

static int pci_config_write(struct pci_bus *bus, unsigned int devfn, int where, int size, u32 val)
{
	unsigned int slot = PCI_SLOT(devfn);
	u8 func = PCI_FUNC(devfn);
	u32 address;
	u32 data;
	u32 num = 0;

	if (bus)
		num = bus->number;

	address = (((where & 0xF00) >> 8) << 24) | (num << 16) | (slot << 11) | (func << 8) | (where & 0xfc) | 0x80000000;
	bridge_w32(address, RALINK_PCI_CONFIG_ADDR);
	data = bridge_r32(RALINK_PCI_CONFIG_DATA_VIRT_REG);

	switch (size) {
	case 1:
		data = (data & ~(0xff << ((where & 3) << 3))) |
			(val << ((where & 3) << 3));
		break;
	case 2:
		data = (data & ~(0xffff << ((where & 3) << 3))) |
			(val << ((where & 3) << 3));
		break;
	case 4:
		data = val;
		break;
	}

	bridge_w32(data, RALINK_PCI_CONFIG_DATA_VIRT_REG);

	return PCIBIOS_SUCCESSFUL;
}

struct pci_ops mt7620_pci_ops= {
	.read	= pci_config_read,
	.write	= pci_config_write,
};

static struct resource mt7620_res_pci_mem1;
static struct resource mt7620_res_pci_io1;
struct pci_controller mt7620_controller = {
	.pci_ops	= &mt7620_pci_ops,
	.mem_resource	= &mt7620_res_pci_mem1,
	.mem_offset	= 0x00000000UL,
	.io_resource	= &mt7620_res_pci_io1,
	.io_offset	= 0x00000000UL,
	.io_map_base	= 0xa0000000,
};

static int mt7620_pci_hw_init(struct platform_device *pdev) {
	/* PCIE: bypass PCIe DLL */
	pcie_phy(0x0, 0x80);
	pcie_phy(0x1, 0x04);

	/* PCIE: Elastic buffer control */
	pcie_phy(0x68, 0xB4);

	pcie_m32(0, BIT(1), RALINK_PCI_PCICFG_ADDR);

	reset_control_assert(rstpcie0);

	rt_sysc_m32(RALINK_PCIE0_CLK_EN, 0, RALINK_CLKCFG1);
	rt_sysc_m32(BIT(19), BIT(31), PPLL_DRV);

	reset_control_deassert(rstpcie0);
	rt_sysc_m32(0, RALINK_PCIE0_CLK_EN, RALINK_CLKCFG1);

	mdelay(100);

	if (!(rt_sysc_r32(PPLL_CFG1) & BIT(23))) {
		dev_err(&pdev->dev, "MT7620 PPLL unlock\n");
		reset_control_assert(rstpcie0);
		rt_sysc_m32(RALINK_PCIE0_CLK_EN, 0, RALINK_CLKCFG1);
		return -1;
	}
	rt_sysc_m32(BIT(18) | BIT(17), BIT(19) | BIT(31), PPLL_DRV);

	return 0;
}

static int mt7628_pci_hw_init(struct platform_device *pdev) {
	u32 val = 0;

	rt_sysc_m32(BIT(16), 0, RALINK_GPIOMODE);
	reset_control_deassert(rstpcie0);
	rt_sysc_m32(0, RALINK_PCIE0_CLK_EN, RALINK_CLKCFG1);
	mdelay(100);

	pcie_m32(~0xff, 0x5, RALINK_PCIEPHY_P0_CTL_OFFSET);

	pci_config_read(NULL, 0, 0x70c, 4, &val);
	val &= ~(0xff) << 8;
	val |= 0x50 << 8;
	pci_config_write(NULL, 0, 0x70c, 4, val);

	pci_config_read(NULL, 0, 0x70c, 4, &val);
	dev_err(&pdev->dev, "Port 0 N_FTS = %x\n", (unsigned int) val);

	return 0;
}

static int mt7620_pci_probe(struct platform_device *pdev)
{
	struct resource *bridge_res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	struct resource *pcie_res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	u32 val = 0;

	rstpcie0 = devm_reset_control_get(&pdev->dev, "pcie0");
	if (IS_ERR(rstpcie0))
		return PTR_ERR(rstpcie0);

	bridge_base = devm_request_and_ioremap(&pdev->dev, bridge_res);
        if (!bridge_base)
		return -ENOMEM;

	pcie_base = devm_request_and_ioremap(&pdev->dev, pcie_res);
        if (!pcie_base)
		return -ENOMEM;

	iomem_resource.start = 0;
	iomem_resource.end = ~0;
	ioport_resource.start = 0;
	ioport_resource.end = ~0;
	printk(KERN_INFO "pci: ralink soc = %X\n",ralink_soc);
	/* bring up the pci core */
	switch (ralink_soc) {
	case MT762X_SOC_MT7620A:
		if (mt7620_pci_hw_init(pdev))
			return -1;
		break;

	case MT762X_SOC_MT7628AN:
		if (mt7628_pci_hw_init(pdev))
			return -1;
		break;

	default:
		printk(KERN_INFO "pcie is not supported on this hardware\n");
		return -1;
	}
	mdelay(50);

	/* enable write access */
	pcie_m32(BIT(1), 0, RALINK_PCI_PCICFG_ADDR);
	mdelay(100);

	/* check if there is a card present */
	if ((pcie_r32(RALINK_PCI0_STATUS) & 0x1) == 0) {
		reset_control_assert(rstpcie0);
		rt_sysc_m32(RALINK_PCIE0_CLK_EN, 0, RALINK_CLKCFG1);
		if (ralink_soc == MT762X_SOC_MT7620A)
			rt_sysc_m32(LC_CKDRVPD_, PDRV_SW_SET, PPLL_DRV);
		dev_err(&pdev->dev, "PCIE0 no card, disable it(RST&CLK)\n");
		return -1;
	}

	/* setup ranges */
	bridge_w32(0xffffffff, RALINK_PCI_MEMBASE);
	bridge_w32(RALINK_PCI_IO_MAP_BASE, RALINK_PCI_IOBASE);

	pcie_w32(0x7FFF0001, RALINK_PCI0_BAR0SETUP_ADDR);
	pcie_w32(MEMORY_BASE, RALINK_PCI0_IMBASEBAR0_ADDR);
	pcie_w32(0x06040001, RALINK_PCI0_CLASS);

	/* enable interrupts */
	pcie_m32(0, BIT(20), RALINK_PCI_PCIMSK_ADDR);

	/* voodoo from the SDK driver */
	pci_config_read(NULL, 0, 4, 4, &val);
	pci_config_write(NULL, 0, 4, 4, val | 0x7);

	pci_load_of_ranges(&mt7620_controller, pdev->dev.of_node);
	register_pci_controller(&mt7620_controller);

	return 0;
}

int __init pcibios_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	u16 cmd;
	u32 val;
	int irq = 0;

	if ((dev->bus->number == 0) && (slot == 0)) {
		pcie_w32(0x7FFF0001, RALINK_PCI0_BAR0SETUP_ADDR); //open 7FFF:2G; ENABLE
		pci_config_write(dev->bus, 0, PCI_BASE_ADDRESS_0, 4, MEMORY_BASE);
		pci_config_read(dev->bus, 0, PCI_BASE_ADDRESS_0, 4, &val);
	} else if ((dev->bus->number == 1) && (slot == 0x0)) {
		irq = RALINK_INT_PCIE0;
	} else {
		dev_err(&dev->dev, "no irq found - bus=0x%x, slot = 0x%x\n", dev->bus->number, slot);
		return 0;
	}
	dev_err(&dev->dev, "card - bus=0x%x, slot = 0x%x irq=%d\n", dev->bus->number, slot, irq);

	pci_write_config_byte(dev, PCI_CACHE_LINE_SIZE, 0x14);  //configure cache line size 0x14
	pci_write_config_byte(dev, PCI_LATENCY_TIMER, 0xFF);  //configure latency timer 0x10
	pci_read_config_word(dev, PCI_COMMAND, &cmd);

	// FIXME
	cmd = cmd | PCI_COMMAND_MASTER | PCI_COMMAND_IO | PCI_COMMAND_MEMORY;
	pci_write_config_word(dev, PCI_COMMAND, cmd);
	pci_write_config_byte(dev, PCI_INTERRUPT_LINE, dev->irq);
	//pci_write_config_byte(dev, PCI_INTERRUPT_PIN, dev->irq);

	return irq;
}

int pcibios_plat_dev_init(struct pci_dev *dev)
{
	return 0;
}

static const struct of_device_id mt7620_pci_ids[] = {
	{ .compatible = "mediatek,mt7620-pci" },
	{},
};
MODULE_DEVICE_TABLE(of, mt7620_pci_ids);

static struct platform_driver mt7620_pci_driver = {
	.probe = mt7620_pci_probe,
	.driver = {
		.name = "mt7620-pci",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(mt7620_pci_ids),
	},
};

static int __init mt7620_pci_init(void)
{
	return platform_driver_register(&mt7620_pci_driver);
}

arch_initcall(mt7620_pci_init);
