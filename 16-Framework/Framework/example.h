#ifndef __EXAMPLE_H__
#define __EXAMPLE_H__

#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/mutex.h>

struct example_data;

struct example_operations {
	ssize_t (*example_read)(struct example_data *edata, const char __user *buf, size_t size);
	ssize_t (*example_write)(struct example_data *edata, const char __user *buf, size_t size);
	int (*func1)(void *example_device);
	int (*func2)(void *example_device);
	int (*func3)(void *example_device);
};

struct example_data {
	dev_t devt;
	void *example_device;
	struct list_head device_entry;
	struct example_operations *ops;
	uint8_t *tx_buf;
	uint8_t *rx_buf;
	uint16_t tx_buflen;
	uint16_t rx_buflen;
	uint16_t bufmaxlen;
	uint16_t users;
	struct mutex buf_lock;
};

struct example_driver {
	char *name;
	int major;
	int minor_start;
	int num; // Max number of minor
	struct cdev example_cdev;
	struct class *example_class;
	struct module *owner;
};

#endif
