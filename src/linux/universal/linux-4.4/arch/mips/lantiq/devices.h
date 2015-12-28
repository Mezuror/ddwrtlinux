/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 */

#ifndef _LTQ_DEVICES_H__
#define _LTQ_DEVICES_H__

#include <lantiq_platform.h>
#include <linux/mtd/physmap.h>

#define IRQ_RES(resname, irq) \
	{.name = #resname, .start = (irq), .flags = IORESOURCE_IRQ}
#define MEM_RES(resname, adr_start, adr_size) \
	{ .name = resname, .flags = IORESOURCE_MEM, \
	  .start = ((adr_start) & ~KSEG1), \
	  .end = ((adr_start + adr_size - 1) & ~KSEG1) }

extern void ltq_register_nor(struct physmap_flash_data *data);
extern void ltq_register_wdt(void);
extern void ltq_register_asc(int port);
extern void ltq_register_pci(struct ltq_pci_data *data);
extern void ltq_register_tapi(void);

#endif
