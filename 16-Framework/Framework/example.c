#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/slab.h>

#include "example.h"

MODULE_LICENSE("Dual BSD/GPL");

static LIST_HEAD(device_list);
static DEFINE_MUTEX(device_list_lock);

#define EXAMPLE_BUFLEN	256

static int file_open(struct inode *inode, struct file *filp) {
	struct example_data *edata;
	int status = -ENXIO;

	printk(KERN_DEBUG "EXAMPLE: open\n");
	
	mutex_lock(&device_list_lock);
	/* Use device_entry to find the example date with matched dev_t in inode. */
	list_for_each_entry(edata, &device_list, device_entry) {
		if(edata->devt == inode->i_rdev) {
			status = 0;
			break;
		}
	}

	if(status) {
		pr_debug("EXAMPLE: nothing for minor %d\n", iminor(inode));
		goto err_find_dev;
	}

	/* Have the RX/TX memory buffer. */
	if(!(edata->rx_buf)) {
		edata->rx_buf = kzalloc(EXAMPLE_BUFLEN, GFP_KERNEL);
		if(!(edata->rx_buf)) {
			pr_debug("EXAMPLE: no more memory\n");
			status = -ENOMEM;
			goto err_find_dev;
		}
	}
	if(!(edata->tx_buf)) {
		edata->tx_buf = kzalloc(EXAMPLE_BUFLEN, GFP_KERNEL);
		if(!(edata->tx_buf)) {
			pr_debug("EXAMPLE: no more memory\n");
			status = -ENOMEM;
			goto err_alloc_tx_buf;
		}
	}
	edata->rx_buflen = 0;
	edata->tx_buflen = 0;
	edata->bufmaxlen = EXAMPLE_BUFLEN;
	edata->users++;
	mutex_unlock(&device_list_lock);

	/* Map the data location to the file data pointer. */
	filp->private_data = edata;
	nonseekable_open(inode, filp);

	return 0;

err_alloc_tx_buf:
	kfree(edata->rx_buf);
	edata->rx_buf = NULL;
err_find_dev:
	mutex_unlock(&device_list_lock);
	return status;
}

static int file_close(struct inode *inode, struct file *filp) {
	struct example_data *edata;
	
	printk(KERN_DEBUG "EXAMPLE: close\n");
	
	edata = filp->private_data;

	mutex_lock(&device_list_lock);
	filp->private_data = NULL;
	
	if(edata->users > 0)
		edata->users--;
	
	/* Last close */
	if(edata->users == 0) {
		kfree(edata->rx_buf);
		kfree(edata->tx_buf);
		edata->rx_buf = NULL;
		edata->tx_buf = NULL;
	}
	mutex_unlock(&device_list_lock);

	return 0;
}

static ssize_t file_read(struct file *filp, char __user *buf, size_t size, loff_t *f_pos) {
	struct example_data *edata;

	printk(KERN_DEBUG "EXAMPLE: read (size=%zu)\n", size);

	edata = filp->private_data;

	if(edata->ops->example_read != NULL)
		return edata->ops->example_read(edata, buf, size);
	else
		return 0;
}

static ssize_t file_write(struct file *filp, const char __user *buf, size_t size, loff_t *f_pos) {
	struct example_data *edata;

	printk(KERN_DEBUG "EXAMPLE: write (size=%zu)\n", size);

	edata = filp->private_data;

	if(edata->ops->example_write != NULL) {
		return edata->ops->example_write(edata, buf, size);
	}
	else
		return 0;
}

/* Add an example compatible device. */
static int example_device_add(struct example_data *edata) {
	INIT_LIST_HEAD(&(edata->device_entry));

	mutex_lock(&device_list_lock);
	list_add(&(edata->device_entry), &device_list);
	mutex_unlock(&device_list_lock);

	return 0;
}
EXPORT_SYMBOL(example_device_add);

/* Remove an example compatible device. */
static int example_device_remove(struct example_data *edata) {
	mutex_lock(&device_list_lock);
	list_del(&(edata->device_entry));
	mutex_unlock(&device_list_lock);

	return 0;
}
EXPORT_SYMBOL(example_device_remove);

static struct file_operations example_fops = {
	.open = file_open,
	.release = file_close,
	.read = file_read,
	.write = file_write,
};

/* Register there is a kind of example driver. */
static int example_register_driver(struct example_driver *driver) {
	dev_t dev;
	int alloc_ret, cdev_err;

	printk(KERN_DEBUG "EXAMPLE: register %s\n", driver->name);

	/* Allocate a character device. */
	alloc_ret = alloc_chrdev_region(&dev,
									driver->minor_start,
									driver->num,
									driver->name);
	if(alloc_ret) {
		printk(KERN_DEBUG "EXAMPLE: Failed to allocate a character device\n");
		return alloc_ret;
	}
	/* Initial the character device driver. */
	driver->major = MAJOR(dev);
	cdev_init(&(driver->example_cdev), &example_fops);
	driver->example_cdev.owner = driver->owner;
	/* Add the character device driver into system. */
	cdev_err = cdev_add(&(driver->example_cdev), dev, driver->num);
	if(cdev_err) {
		printk(KERN_DEBUG "EXAMPLE: Failed to register a character device\n");
		/* Release the allocated character device. */
		if(alloc_ret == 0) {
			unregister_chrdev_region(dev, driver->num);
		}
		return cdev_err;
	}
	printk(KERN_DEBUG "EXAMPLE: %s driver(major %d) installed.\n", driver->name, driver->major);

	/* Create device class. */
	driver->example_class = class_create(driver->owner, driver->name);
	if(IS_ERR(driver->example_class)) {
		printk(KERN_DEBUG "EXAMPLE: Failed to create a class of device.\n");
		/* Release the added character device. */
		if(cdev_err == 0)
			cdev_del(&(driver->example_cdev));
		/* Release the allocated character device. */
		if(alloc_ret == 0)
			unregister_chrdev_region(dev, driver->num);
		return -1;
	}
	printk(KERN_DEBUG "EXAMPLE: %s class created.\n", driver->name);

	return 0;
}
EXPORT_SYMBOL(example_register_driver);

/* Unregister the example driver. */
static int example_unregister_driver(struct example_driver *driver) {
	dev_t dev = MKDEV(driver->major, driver->minor_start);
	
	printk(KERN_DEBUG "EXAMPLE: unregister %s\n", driver->name);
	/* Delete device class. */
	class_destroy(driver->example_class);
	/* Delete the character device driver from system. */
	cdev_del(&(driver->example_cdev));
	/* Unregister the allocated character device. */
	unregister_chrdev_region(dev, driver->num);
	printk(KERN_DEBUG "EXAMPLE: %s driver removed.\n", driver->name);

	return 0;
}
EXPORT_SYMBOL(example_unregister_driver);
