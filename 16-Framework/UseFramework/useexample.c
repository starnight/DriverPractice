#include <linux/init.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

#include "useexample.h"
#include "somedevice.h"

#define N_EXAMPLE_MINORS	4

static DECLARE_BITMAP(minors, N_EXAMPLE_MINORS);

static DEFINE_MUTEX(minors_lock);

static ssize_t useexample_read(struct example_data *edata, const char __user *buf, size_t size) {
	size_t len = 0, oblen;
	int res;
	struct some_device *esd;

	esd = edata->example_device;
	printk(KERN_DEBUG "USEEXAMPLE: some device #%d read\n", esd->address);

	mutex_lock(&(edata->buf_lock));
	/* Read from the some device into the example data's RX buffer. */
	res = edata->bufmaxlen - edata->rx_buflen;
	len = (res <= size) ? res : size;
	len = somedevice_read(esd, edata->rx_buf + edata->rx_buflen, len);
	oblen = edata->rx_buflen;
	edata->rx_buflen += len;

	/* Read from the example data into user space. */
	copy_to_user((void *)buf, edata->rx_buf + oblen, len);
	edata->rx_buflen -= len;

	mutex_unlock(&(edata->buf_lock));

	return len;
}

static ssize_t useexample_write(struct example_data *edata, const char __user *buf, size_t size) {
	ssize_t c = 0;
	size_t len, oblen;
	int res;
	struct some_device *esd;

	esd = edata->example_device;
	printk(KERN_DEBUG "USEEXAMPLE: some device #%d write\n", esd->address);

	mutex_lock(&(edata->buf_lock));
	while(size > 0) {
		/* Write from user space into the example data. */
		res = edata->bufmaxlen - edata->tx_buflen;
		len = (res <= size) ? res : size;
		if(len == 0)
			break;
		copy_from_user(edata->tx_buf + edata->tx_buflen, buf + c, len);
		oblen = edata->tx_buflen;
		edata->tx_buflen += len;

		/* Write from example data into some device. */
		res = somedevice_write(esd, edata->tx_buf + oblen, edata->tx_buflen);
		edata->tx_buflen -= len;
		if(res <= 0) {
			break;
		}
		else if(res < len) {
			c += res;
			break;
		}
		c += len;
		size -= len;
	}
	mutex_unlock(&(edata->buf_lock));

	return c;
}

struct example_driver driver = {
	.name = "useexample",
	.num = N_EXAMPLE_MINORS,
	.owner = THIS_MODULE,
};

struct example_operations eops = {
	.example_read = useexample_read,
	.example_write = useexample_write,
};


/* Emulate the some device's probe callback function. */
static int emulate_probe(struct some_device *sd) {
	struct example_data *edata;
	unsigned long minor;
	int status;

	printk(KERN_DEBUG "USEEXAMPLE: probe a some device with address %d\n", sd->address);

	/* Allocate example device's data. */
	edata = kzalloc(sizeof(struct example_data), GFP_KERNEL);
	if(!edata)
		return -ENOMEM;

	/* Initial the example device's data. */
	edata->example_device = sd;
	edata->ops = &eops;
	mutex_lock(&minors_lock);
	minor = find_first_zero_bit(minors, N_EXAMPLE_MINORS);
	if(minor < N_EXAMPLE_MINORS) {
		set_bit(minor, minors);
		edata->devt = MKDEV(driver.major, minor);
		device_create(driver.example_class,
					  NULL,
					  edata->devt,
					  edata,
					  "useexample%d", sd->address);
		/* Set the some device's driver data for later use.  */
		somedevice_set_drvdata(sd, edata);
		example_device_add(edata);
		status = 0;
	}
	else {
		/* No more example device available. */
		kfree(edata);
		status = -ENODEV;
	}
	mutex_unlock(&minors_lock);

	return status;
}

/* Emulate the some device's remove callback function. */
static int emulate_remove(struct some_device *sd) {
	struct example_data *edata;
	
	printk(KERN_DEBUG "USEEXAMPLE: remove a some device with address %d", sd->address);

	edata = somedevice_get_drvdata(sd);

	/* Clear the example device's data. */
	edata->example_device = NULL;
	/* No more operations to the example device from user space. */
	example_device_remove(edata);
	mutex_lock(&minors_lock);
	device_destroy(driver.example_class, edata->devt);
	clear_bit(MINOR(edata->devt), minors);
	mutex_unlock(&minors_lock);

	/* Free the memory of the example device.  */
	kfree(edata);
	
	return 0;
}

/* Emulate there are N_EXAMPLE_MINORS some devices. */
static struct some_device esd[N_EXAMPLE_MINORS] = {
	{ .bus = NULL, .address = 0, .buflen = 0, .bufmaxlen = SOME_DEVICE_BUF_LEN },
	{ .bus = NULL, .address = 1, .buflen = 0, .bufmaxlen = SOME_DEVICE_BUF_LEN },
	{ .bus = NULL, .address = 2, .buflen = 0, .bufmaxlen = SOME_DEVICE_BUF_LEN },
	{ .bus = NULL, .address = 3, .buflen = 0, .bufmaxlen = SOME_DEVICE_BUF_LEN } };

/* UseExample kernel module's initial function. */
static int useexample_init(void) {
	int status;
	
	printk(KERN_DEBUG "USEEXAMPLE: init\n");
	
	/* Register a kind of example driver. */
	status = example_register_driver(&driver);

	if(status == 0) {
		/* Emulate that the some devices are probed. */
		int i;
		for(i = 0; i < N_EXAMPLE_MINORS; i++)
			emulate_probe(esd + i);
	}

	return status;
}

/* UseExample kernel module's exit function. */
static void useexample_exit(void) {
	int i;
	
	printk(KERN_DEBUG "USEEXAMPLE: exit\n");

	/* Emulate the the some devices are removed. */
	for(i = 0; i < N_EXAMPLE_MINORS; i++)
		emulate_remove(esd + i);

	/* Unregister the example driver. */
	example_unregister_driver(&driver);
}

module_init(useexample_init);
module_exit(useexample_exit);

MODULE_LICENSE("Dual BSD/GPL");
