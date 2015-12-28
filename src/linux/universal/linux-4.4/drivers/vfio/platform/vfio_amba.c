/*
 * Copyright (C) 2013 - Virtual Open Systems
 * Author: Antonios Motakis <a.motakis@virtualopensystems.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/slab.h>
#include <linux/vfio.h>
#include <linux/amba/bus.h>

#include "vfio_platform_private.h"

#define DRIVER_VERSION  "0.10"
#define DRIVER_AUTHOR   "Antonios Motakis <a.motakis@virtualopensystems.com>"
#define DRIVER_DESC     "VFIO for AMBA devices - User Level meta-driver"

/* probing devices from the AMBA bus */

static struct resource *get_amba_resource(struct vfio_platform_device *vdev,
					  int i)
{
	struct amba_device *adev = (struct amba_device *) vdev->opaque;

	if (i == 0)
		return &adev->res;

	return NULL;
}

static int get_amba_irq(struct vfio_platform_device *vdev, int i)
{
	struct amba_device *adev = (struct amba_device *) vdev->opaque;
	int ret = 0;

	if (i < AMBA_NR_IRQS)
		ret = adev->irq[i];

	/* zero is an unset IRQ for AMBA devices */
	return ret ? ret : -ENXIO;
}

static int vfio_amba_probe(struct amba_device *adev, const struct amba_id *id)
{
	struct vfio_platform_device *vdev;
	int ret;

	vdev = kzalloc(sizeof(*vdev), GFP_KERNEL);
	if (!vdev)
		return -ENOMEM;

	vdev->name = kasprintf(GFP_KERNEL, "vfio-amba-%08x", adev->periphid);
	if (!vdev->name) {
		kfree(vdev);
		return -ENOMEM;
	}

	vdev->opaque = (void *) adev;
	vdev->flags = VFIO_DEVICE_FLAGS_AMBA;
	vdev->get_resource = get_amba_resource;
	vdev->get_irq = get_amba_irq;
	vdev->parent_module = THIS_MODULE;

	ret = vfio_platform_probe_common(vdev, &adev->dev);
	if (ret) {
		kfree(vdev->name);
		kfree(vdev);
	}

	return ret;
}

static int vfio_amba_remove(struct amba_device *adev)
{
	struct vfio_platform_device *vdev;

	vdev = vfio_platform_remove_common(&adev->dev);
	if (vdev) {
		kfree(vdev->name);
		kfree(vdev);
		return 0;
	}

	return -EINVAL;
}

static struct amba_id pl330_ids[] = {
	{ 0, 0 },
};

MODULE_DEVICE_TABLE(amba, pl330_ids);

static struct amba_driver vfio_amba_driver = {
	.probe = vfio_amba_probe,
	.remove = vfio_amba_remove,
	.id_table = pl330_ids,
	.drv = {
		.name = "vfio-amba",
		.owner = THIS_MODULE,
	},
};

module_amba_driver(vfio_amba_driver);

MODULE_VERSION(DRIVER_VERSION);
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
