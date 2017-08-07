/*
 * Copyright (C) 2004 IBM Corporation
 * Copyright (C) 2014 Intel Corporation
 *
 * Authors:
 * Jarkko Sakkinen <jarkko.sakkinen@linux.intel.com>
 * Leendert van Doorn <leendert@watson.ibm.com>
 * Dave Safford <safford@watson.ibm.com>
 * Reiner Sailer <sailer@watson.ibm.com>
 * Kylene Hall <kjhall@us.ibm.com>
 *
 * Maintained by: <tpmdd-devel@lists.sourceforge.net>
 *
 * TPM chip management routines.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, version 2 of the
 * License.
 *
 */

#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/freezer.h>
#include <linux/major.h>
#include "tpm.h"
#include "tpm_eventlog.h"

DEFINE_IDR(dev_nums_idr);
static DEFINE_MUTEX(idr_lock);

struct class *tpm_class;
dev_t tpm_devt;

/**
 * tpm_try_get_ops() - Get a ref to the tpm_chip
 * @chip: Chip to ref
 *
 * The caller must already have some kind of locking to ensure that chip is
 * valid. This function will lock the chip so that the ops member can be
 * accessed safely. The locking prevents tpm_chip_unregister from
 * completing, so it should not be held for long periods.
 *
 * Returns -ERRNO if the chip could not be got.
 */
int tpm_try_get_ops(struct tpm_chip *chip)
{
	int rc = -EIO;

	get_device(&chip->dev);

	down_read(&chip->ops_sem);
	if (!chip->ops)
		goto out_lock;

	if (!try_module_get(chip->dev.parent->driver->owner))
		goto out_lock;

	return 0;
out_lock:
	up_read(&chip->ops_sem);
	put_device(&chip->dev);
	return rc;
}
EXPORT_SYMBOL_GPL(tpm_try_get_ops);

/**
 * tpm_put_ops() - Release a ref to the tpm_chip
 * @chip: Chip to put
 *
 * This is the opposite pair to tpm_try_get_ops(). After this returns chip may
 * be kfree'd.
 */
void tpm_put_ops(struct tpm_chip *chip)
{
	module_put(chip->dev.parent->driver->owner);
	up_read(&chip->ops_sem);
	put_device(&chip->dev);
}
EXPORT_SYMBOL_GPL(tpm_put_ops);

/**
 * tpm_chip_find_get() - return tpm_chip for a given chip number
 * @chip_num: id to find
 *
 * The return'd chip has been tpm_try_get_ops'd and must be released via
 * tpm_put_ops
  */
struct tpm_chip *tpm_chip_find_get(int chip_num)
{
	struct tpm_chip *chip, *res = NULL;
	int chip_prev;

	mutex_lock(&idr_lock);

	if (chip_num == TPM_ANY_NUM) {
		chip_num = 0;
		do {
			chip_prev = chip_num;
			chip = idr_get_next(&dev_nums_idr, &chip_num);
			if (chip && !tpm_try_get_ops(chip)) {
				res = chip;
				break;
			}
		} while (chip_prev != chip_num);
	} else {
		chip = idr_find_slowpath(&dev_nums_idr, chip_num);
		if (chip && !tpm_try_get_ops(chip))
			res = chip;
	}

	mutex_unlock(&idr_lock);

	return res;
}

/**
 * tpm_dev_release() - free chip memory and the device number
 * @dev: the character device for the TPM chip
 *
 * This is used as the release function for the character device.
 */
static void tpm_dev_release(struct device *dev)
{
	struct tpm_chip *chip = container_of(dev, struct tpm_chip, dev);

	mutex_lock(&idr_lock);
	idr_remove(&dev_nums_idr, chip->dev_num);
	mutex_unlock(&idr_lock);

	kfree(chip);
}


/**
 * tpm_class_shutdown() - prepare the TPM device for loss of power.
 * @dev: device to which the chip is associated.
 *
 * Issues a TPM2_Shutdown command prior to loss of power, as required by the
 * TPM 2.0 spec.
 * Then, calls bus- and device- specific shutdown code.
 *
 * XXX: This codepath relies on the fact that sysfs is not enabled for
 * TPM2: sysfs uses an implicit lock on chip->ops, so this could race if TPM2
 * has sysfs support enabled before TPM sysfs's implicit locking is fixed.
 */
static int tpm_class_shutdown(struct device *dev)
{
	struct tpm_chip *chip = container_of(dev, struct tpm_chip, dev);

	if (chip->flags & TPM_CHIP_FLAG_TPM2) {
		down_write(&chip->ops_sem);
		tpm2_shutdown(chip, TPM2_SU_CLEAR);
		chip->ops = NULL;
		up_write(&chip->ops_sem);
	}
	/* Allow bus- and device-specific code to run. Note: since chip->ops
	 * is NULL, more-specific shutdown code will not be able to issue TPM
	 * commands.
	 */
	if (dev->bus && dev->bus->shutdown)
		dev->bus->shutdown(dev);
	else if (dev->driver && dev->driver->shutdown)
		dev->driver->shutdown(dev);
	return 0;
}


/**
 * tpmm_chip_alloc() - allocate a new struct tpm_chip instance
 * @dev: device to which the chip is associated
 * @ops: struct tpm_class_ops instance
 *
 * Allocates a new struct tpm_chip instance and assigns a free
 * device number for it. Caller does not have to worry about
 * freeing the allocated resources. When the devices is removed
 * devres calls tpmm_chip_remove() to do the job.
 */
struct tpm_chip *tpmm_chip_alloc(struct device *dev,
				 const struct tpm_class_ops *ops)
{
	struct tpm_chip *chip;
	int rc;

	chip = kzalloc(sizeof(*chip), GFP_KERNEL);
	if (chip == NULL)
		return ERR_PTR(-ENOMEM);

	mutex_init(&chip->tpm_mutex);
	init_rwsem(&chip->ops_sem);

	chip->ops = ops;

	mutex_lock(&idr_lock);
	rc = idr_alloc(&dev_nums_idr, NULL, 0, TPM_NUM_DEVICES, GFP_KERNEL);
	mutex_unlock(&idr_lock);
	if (rc < 0) {
		dev_err(dev, "No available tpm device numbers\n");
		kfree(chip);
		return ERR_PTR(rc);
	}
	chip->dev_num = rc;

	scnprintf(chip->devname, sizeof(chip->devname), "tpm%d", chip->dev_num);

	dev_set_drvdata(dev, chip);

	chip->dev.class = tpm_class;
	chip->dev.class->shutdown = tpm_class_shutdown;
	chip->dev.release = tpm_dev_release;
	chip->dev.parent = dev;
#ifdef CONFIG_ACPI
	chip->dev.groups = chip->groups;
#endif

	if (chip->dev_num == 0)
		chip->dev.devt = MKDEV(MISC_MAJOR, TPM_MINOR);
	else
		chip->dev.devt = MKDEV(MAJOR(tpm_devt), chip->dev_num);

	dev_set_name(&chip->dev, "%s", chip->devname);

	device_initialize(&chip->dev);

	cdev_init(&chip->cdev, &tpm_fops);
	chip->cdev.owner = dev->driver->owner;
	chip->cdev.kobj.parent = &chip->dev.kobj;

	devm_add_action(dev, (void (*)(void *)) put_device, &chip->dev);

	return chip;
}
EXPORT_SYMBOL_GPL(tpmm_chip_alloc);

static int tpm_add_char_device(struct tpm_chip *chip)
{
	int rc;

	rc = cdev_add(&chip->cdev, chip->dev.devt, 1);
	if (rc) {
		dev_err(&chip->dev,
			"unable to cdev_add() %s, major %d, minor %d, err=%d\n",
			chip->devname, MAJOR(chip->dev.devt),
			MINOR(chip->dev.devt), rc);

		return rc;
	}

	rc = device_add(&chip->dev);
	if (rc) {
		dev_err(&chip->dev,
			"unable to device_register() %s, major %d, minor %d, err=%d\n",
			chip->devname, MAJOR(chip->dev.devt),
			MINOR(chip->dev.devt), rc);

		cdev_del(&chip->cdev);
		return rc;
	}

	/* Make the chip available. */
	mutex_lock(&idr_lock);
	idr_replace(&dev_nums_idr, chip, chip->dev_num);
	mutex_unlock(&idr_lock);

	return rc;
}

static void tpm_del_char_device(struct tpm_chip *chip)
{
	cdev_del(&chip->cdev);
	device_del(&chip->dev);

	/* Make the chip unavailable. */
	mutex_lock(&idr_lock);
	idr_replace(&dev_nums_idr, NULL, chip->dev_num);
	mutex_unlock(&idr_lock);

	/* Make the driver uncallable. */
	down_write(&chip->ops_sem);
	chip->ops = NULL;
	up_write(&chip->ops_sem);
}

static int tpm1_chip_register(struct tpm_chip *chip)
{
	int rc;

	if (chip->flags & TPM_CHIP_FLAG_TPM2)
		return 0;

	rc = tpm_sysfs_add_device(chip);
	if (rc)
		return rc;

	chip->bios_dir = tpm_bios_log_setup(chip->devname);

	return 0;
}

static void tpm1_chip_unregister(struct tpm_chip *chip)
{
	if (chip->flags & TPM_CHIP_FLAG_TPM2)
		return;

	if (chip->bios_dir)
		tpm_bios_log_teardown(chip->bios_dir);

	tpm_sysfs_del_device(chip);
}

/*
 * tpm_chip_register() - create a character device for the TPM chip
 * @chip: TPM chip to use.
 *
 * Creates a character device for the TPM chip and adds sysfs attributes for
 * the device. As the last step this function adds the chip to the list of TPM
 * chips available for in-kernel use.
 *
 * This function should be only called after the chip initialization is
 * complete.
 */
int tpm_chip_register(struct tpm_chip *chip)
{
	int rc;

	rc = tpm1_chip_register(chip);
	if (rc)
		return rc;

	tpm_add_ppi(chip);

	rc = tpm_add_char_device(chip);
	if (rc)
		goto out_err;

	chip->flags |= TPM_CHIP_FLAG_REGISTERED;

	if (!(chip->flags & TPM_CHIP_FLAG_TPM2)) {
		rc = __compat_only_sysfs_link_entry_to_kobj(
		    &chip->dev.parent->kobj, &chip->dev.kobj, "ppi");
		if (rc && rc != -ENOENT) {
			tpm_chip_unregister(chip);
			return rc;
		}
	}

	return 0;
out_err:
	tpm1_chip_unregister(chip);
	return rc;
}
EXPORT_SYMBOL_GPL(tpm_chip_register);

/*
 * tpm_chip_unregister() - release the TPM driver
 * @chip: TPM chip to use.
 *
 * Takes the chip first away from the list of available TPM chips and then
 * cleans up all the resources reserved by tpm_chip_register().
 *
 * Once this function returns the driver call backs in 'op's will not be
 * running and will no longer start.
 *
 * NOTE: This function should be only called before deinitializing chip
 * resources.
 */
void tpm_chip_unregister(struct tpm_chip *chip)
{
	if (!(chip->flags & TPM_CHIP_FLAG_REGISTERED))
		return;

	if (!(chip->flags & TPM_CHIP_FLAG_TPM2))
		sysfs_remove_link(&chip->dev.parent->kobj, "ppi");

	tpm1_chip_unregister(chip);
	tpm_del_char_device(chip);
}
EXPORT_SYMBOL_GPL(tpm_chip_unregister);
