#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/stat.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/errno.h>
#include "test_ioctl.h"

MODULE_LICENSE("Dual BSD/GPL");

/* The device packaged data and registers structure. */
struct device_data_reg {
	int flag;
	char *data;
	int len;
};

/* The device initial data and registers' value. */
static struct device_data_reg dev_data = {
									.flag = 0,
									.len = 0};

/* The file handler for the file going to be opened. */
struct file* ch_dev = NULL;
/* The file going to be opened. */
#define FILE_DIR	"dummy.txt"

static int example_open(struct inode *inode, struct file *filp) {
	int err = 0;
	struct kstat st;

	printk(KERN_DEBUG "EXAMPLE: open\n");

	/* Map the data location to the file data pointer. */
	filp->private_data = &dev_data;

	/* Open the file with reading and writing in file system. */
	ch_dev = filp_open(FILE_DIR, O_RDWR, 0666);

	if(IS_ERR(ch_dev)) {
		err = PTR_ERR(ch_dev);
	}
	else {
		/* Have the file's size. */
		err = vfs_getattr(&(ch_dev->f_path), &st);
		dev_data.len = st.size;
	}

	return err;
}

static int example_close(struct inode *inode, struct file *filp) {
	printk(KERN_DEBUG "EXAMPLE: close\n");
	
	/* Close the opened file in this module. */
	if(ch_dev != NULL)
		filp_close(ch_dev, NULL);

	/* Release the mapping of file data address. */
	if(filp->private_data) {
		filp->private_data = NULL;
	}

	return 0;
}

static ssize_t example_read(struct file *filp, char __user *buf, size_t size, loff_t *f_pos) {
	size_t count;
	uint8_t byte;
	struct device_data_reg *data_p;

	mm_segment_t old_fs;
	ssize_t ret;
	loff_t tmp_pos;
	
	printk(KERN_DEBUG "EXAMPLE: read (size=%zu)\n", size);

	data_p = filp->private_data;

	/* Ask kernel do not check the memory boundary. */
	old_fs = get_fs();
	set_fs(get_ds());

	/* Read from the device data to user space. */
	for(count = 0; (count < size) && (*f_pos) < data_p->len; ++(*f_pos), ++count) {
		/* Read 1 byte from the opened file. */
		tmp_pos = *f_pos;
		ret = vfs_read(ch_dev, &byte, 1, &tmp_pos);
		/* Copy the read byte to user program. */
		if((ret >= 0) && (copy_to_user(buf + count, &byte, 1) != 0)) {
			break;
		}
		printk(KERN_DEBUG "EXAMPLE: read (buf[%zu]=%02x '%c')\n",
				count, (unsigned)byte, (char)byte);
	}

	/* Ask kernel to check the memory boundary again. */
	set_fs(old_fs);

	return count;
}

static ssize_t example_write(struct file *filp, const char __user *buf, size_t size, loff_t *f_pos) {
	size_t count;
	ssize_t ret;
	uint8_t byte;
	struct device_data_reg *data_p;
	
	mm_segment_t old_fs;
	loff_t tmp_pos;

	printk(KERN_DEBUG "EXAMPLE: write (size=%zu)\n", size);

	data_p = filp->private_data;

	/* Ask kernel do not check the memory boundary. */
	old_fs = get_fs();
	set_fs(get_ds());

	/* Write from user space to the device. */
	for(count = 0; count < size; ++(*f_pos), ++count) {
		/* Copy 1 byte from user program. */
		if(copy_from_user(&byte, buf + count, 1) != 0) {
			break;
		}
		/* Write the copied byte to the opened file. */
		tmp_pos = *f_pos;
		ret = vfs_write(ch_dev, &byte, 1, &tmp_pos);
		if(ret == 1)
			printk(KERN_DEBUG "EXAMPLE: write (buf[%zu]=%02x)\n", count, (unsigned)byte);
		else
			break;
	}

	/* Ask kernel to check the memory boundary again. */
	set_fs(old_fs);

	if(ret == -1)
		return -1;

	/* Update the opened file size if it began bigger. */
	if((*f_pos) >= data_p->len) {
		data_p->len = *f_pos;
	}
	ret = count;

	return ret;
}

static long example_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
	long ret;
	int *pval;
	struct device_data_reg *data_p;

	ret = 0;
	pval = (int __user *)arg;

	printk(KERN_DEBUG "EXAMPLE: ioctl (cmd=%d)\n", cmd);

	data_p = filp->private_data;

	/* I/O control by each command. */
	switch(cmd) {
		case IOCTL_GET_FLAG:
			if(copy_to_user(pval, &(data_p->flag), sizeof(data_p->flag)))
				ret = -EFAULT;
			else
				printk(KERN_DEBUG "EXAMPLE: ioctl get flag %d\n", data_p->flag);
			break;
		case IOCTL_SET_FLAG:
			if(copy_from_user(&(data_p->flag), pval, sizeof(data_p->flag)))
				ret = -EFAULT;
			else
				printk(KERN_DEBUG "EXAMPLE: ioctl set flag %d\n", data_p->flag);
			break;
		default:
			ret = -ENOTTY;
	}

	return ret;
}

static struct file_operations example_fops = {
	.open = example_open,
	.release = example_close,
	.read = example_read,
	.write = example_write,
	.unlocked_ioctl = example_ioctl,
};

#define EXAMPLE_NAME	"example"

static unsigned int example_major;
static unsigned int example_devs = 2;
static struct cdev example_cdev;
static struct class *example_sys_class = NULL;

static int example_init(void) {
	dev_t dev;
	int alloc_ret, cdev_err;

	printk(KERN_DEBUG "EXAMPLE: init\n");

	/* Allocate a character device. */
	alloc_ret = alloc_chrdev_region(&dev, 0, example_devs, EXAMPLE_NAME);
	if(alloc_ret) {
		printk(KERN_DEBUG "EXAMPLE: Failed to allocate a character device\n");
		return -1;
	}
	/* Initial the character device ddriver. */
	example_major = MAJOR(dev);
	cdev_init(&example_cdev, &example_fops);
	example_cdev.owner = THIS_MODULE;
	/* Add the character device driver into system. */
	dev = MKDEV(example_major, 0);
	cdev_err = cdev_add(&example_cdev, dev, example_devs);
	if(cdev_err) {
		printk(KERN_DEBUG "EXAMPLE: Failed to register a character device\n");
		/* Release the allocated character device. */
		if(alloc_ret == 0) {
			unregister_chrdev_region(dev, example_devs);
		}
		return -1;
	}
	printk(KERN_DEBUG "EXAMPLE: %s driver(major %d) installed.\n", EXAMPLE_NAME, example_major);

	/* Create device class. */
	example_sys_class = class_create(THIS_MODULE, EXAMPLE_NAME);
	if(IS_ERR(example_sys_class)) {
		printk(KERN_DEBUG "EXAMPLE: Failed to create a class of device.\n");
		/* Release the added character device. */
		if(cdev_err == 0)
			cdev_del(&example_cdev);
		/* Release the allocated character device. */
		if(alloc_ret == 0)
			unregister_chrdev_region(dev, example_devs);
		return -1;
	}
	printk(KERN_DEBUG "EXAMPLE: %s class created.\n", EXAMPLE_NAME);

	/* Create device node. */
	device_create(example_sys_class, NULL, dev, NULL, EXAMPLE_NAME);
	printk(KERN_DEBUG "EXAMPLE: %s device node created.\n", EXAMPLE_NAME);

	return 0;
}

static void example_exit(void) {
	dev_t dev = MKDEV(example_major, 0);

	printk(KERN_DEBUG "EXAMPLE: exit\n");
	/* Destory device nodes. */
	device_destroy(example_sys_class, dev);
	/* Delete device class. */
	class_destroy(example_sys_class);
	/* Delete the character device driver from system. */
	cdev_del(&example_cdev);
	/* Unregister the allocated character device. */
	unregister_chrdev_region(dev, example_devs);
	printk(KERN_DEBUG "EXAMPLE: %s driver removed.\n", EXAMPLE_NAME); 
}

module_init(example_init);
module_exit(example_exit);
