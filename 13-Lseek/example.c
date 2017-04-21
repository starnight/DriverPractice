#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/sched.h>
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
	/* Read / Write lock. */
	rwlock_t lock;
	/* Semaphore for poll. */
	struct semaphore sem;
	wait_queue_head_t read_wait;
	wait_queue_head_t write_wait;
	/* Spinlock for read / write timer interrupt. */
	spinlock_t read_splock;
	spinlock_t write_splock;
	/* Timer. */
	struct timer_list read_timeout;
	struct timer_list write_timeout;
	/* Ready to be read / write flag. */
	int read_flag;
	int write_flag;
};

/* The device initial data and registers' value. */
static char _buffer[] = "0123456789\r\n";
static struct device_data_reg dev_data = {
									.flag = 0,
									.data = _buffer,
									.len = 12};

/* Read timer interrupt handler. */
static void example_read_timeout_handler(unsigned long arg) {
	struct device_data_reg *data_p = (struct device_data_reg *)arg;
	unsigned long flags;

	printk(KERN_DEBUG "EXAMPLE: read timer timeout\n");

	/* Spin lock in interrupt. */
	spin_lock_irqsave(&(data_p->read_splock), flags);

	/* File is ready to be read. */
	data_p->read_flag = 1;
	wake_up_interruptible(&(data_p->read_wait));

	/* Release spin lock. */
	spin_unlock_irqrestore(&(data_p->read_splock), flags);
}

/* Write timer interrupt handler. */
static void example_write_timeout_handler(unsigned long arg) {
	struct device_data_reg *data_p = (struct device_data_reg *)arg;
	unsigned long flags;

	printk(KERN_DEBUG "EXAMPLE: write timer timeout\n");

	/* Spin lock in interrupt. */
	spin_lock_irqsave(&(data_p->write_splock), flags);

	/* File is ready to be read. */
	data_p->write_flag = 1;
	wake_up_interruptible(&(data_p->write_wait));

	/* Release spin lock. */
	spin_unlock_irqrestore(&(data_p->write_splock), flags);
}

static int example_open(struct inode *inode, struct file *filp) {
	printk(KERN_DEBUG "EXAMPLE: open\n");

	/* Map the data location to the file data pointer. */
	filp->private_data = &dev_data;
	/* Initial lock. */
	rwlock_init(&(dev_data.lock));
	/* Initial the semaphore for multiplexing check. */
	sema_init(&(dev_data.sem), 1);
	/* Initial the wait queue head. */
	init_waitqueue_head(&(dev_data.read_wait));
	init_waitqueue_head(&(dev_data.write_wait));
	/* Initial the spin locks for read / write timer interrupt. */
	spin_lock_init(&(dev_data.read_splock));
	spin_lock_init(&(dev_data.write_splock));
	/* initial the read / write timers. */
	init_timer(&(dev_data.read_timeout));
	init_timer(&(dev_data.write_timeout));
	dev_data.read_timeout.function = example_read_timeout_handler;
	dev_data.write_timeout.function = example_write_timeout_handler;
	dev_data.read_timeout.data = (unsigned long)(&dev_data);
	dev_data.write_timeout.data = (unsigned long)(&dev_data);
	mod_timer(&(dev_data.read_timeout), jiffies + 2*HZ);
	mod_timer(&(dev_data.write_timeout), jiffies + 2*HZ);	
	dev_data.read_flag = 0;
	dev_data.write_flag = 0;

	return 0;
}

static int example_close(struct inode *inode, struct file *filp) {
	struct device_data_reg *data_p;

	printk(KERN_DEBUG "EXAMPLE: close\n");

	data_p = filp->private_data;
	/* Release the mapping of file data address. */
	if(filp->private_data) {
		del_timer_sync(&(data_p->read_timeout));
		del_timer_sync(&(data_p->write_timeout));
		filp->private_data = NULL;
	}

	return 0;
}

static ssize_t example_read(struct file *filp, char __user *buf, size_t size, loff_t *f_pos) {
	size_t count;
	uint8_t byte;
	struct device_data_reg *data_p;
	
	printk(KERN_DEBUG "EXAMPLE: read (size=%zu)\n", size);

	data_p = filp->private_data;
	/* Get the lock for reading. */
	read_lock(&(data_p->lock));
	/* Read from the device data to user space. */
	for(count = 0; (count < size) && (*f_pos) < data_p->len; ++(*f_pos), ++count) {
		byte = data_p->data[*f_pos];
		if(copy_to_user(buf + count, &byte, 1) != 0) {
			break;
		}
		printk(KERN_DEBUG "EXAMPLE: read (buf[%zu]=%02x)\n", count, (unsigned)byte);
	}
	/* Release the lock for reading. */
	read_unlock(&(data_p->lock));

	return count;
}

static ssize_t example_write(struct file *filp, const char __user *buf, size_t size, loff_t *f_pos) {
	size_t count;
	ssize_t ret;
	uint8_t byte;
	struct device_data_reg *data_p;

	printk(KERN_DEBUG "EXAMPLE: write (size=%zu)\n", size);

	data_p = filp->private_data;
	/* Get the lock for writing. */
	write_lock(&(data_p->lock));
	/* Write from user space to the device. */
	for(count = 0; (count < size) && (*f_pos) < data_p->len; ++(*f_pos), ++count) {
		if(copy_from_user(&byte, buf + count, 1) != 0) {
			break;
		}
		data_p->data[*f_pos] = byte;
		printk(KERN_DEBUG "EXAMPLE: write (buf[%zu]=%02x)\n", count, (unsigned)byte);
	}
	/* Release the lock for writing. */
	write_unlock(&(data_p->lock));

	if((count == 0) && ((*f_pos) >= data_p->len)) {
		ret = -ENOBUFS;
	}
	else {
		ret = count;
	}

	return ret;
}

static loff_t example_llseek(struct file *filp, loff_t off, int whence) {
	loff_t new_pos;
	struct device_data_reg *data_p;

	printk(KERN_DEBUG "EXAMPLE: llseek (offset=%d)\n", (int)off);

	data_p = filp->private_data;
	switch(whence) {
		case SEEK_SET:	/* Seek from the start of the file. */
			new_pos = off;
			break;
		case SEEK_CUR:	/* Seek from the start of the file. */
			new_pos = filp->f_pos + off;
			break;
		case SEEK_END:	/* Seek from the start of the file. */
			new_pos = data_p->len + off;
			break;
		default:
			return -EINVAL;
	}

	/* Make sure the new offset is in the range. */
	if((new_pos < 0) || (data_p->len < new_pos)) {
		return -ENOBUFS;
	}
	else {
		filp->f_pos = new_pos;
		return new_pos;
	}
}

static long example_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
	long ret;
	int *pval;
	struct device_data_reg *data_p;

	ret = 0;
	pval = (int __user *)arg;

	printk(KERN_DEBUG "EXAMPLE: unlocked_ioctl (cmd=%d)\n", cmd);

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

static unsigned int example_poll(struct file *filp, struct poll_table_struct *wait) {
	struct device_data_reg *data_p;
	unsigned int mask;

	printk(KERN_DEBUG "EXAMPLE: poll\n");

	data_p = filp->private_data;
	mask = 0;

	if(data_p == NULL)
		return -EBADFD;

	/* Decrease semaphore. */
	down(&(data_p->sem));
	/* Register a file descriptor into wait queue for reading. */
	poll_wait(filp, &(data_p->read_wait), wait);
	poll_wait(filp, &(data_p->write_wait), wait);
	/* Check ready to be read / written. */
	if(data_p->read_flag == 1) {
		mask |= POLLIN | POLLRDNORM;
	}
	if(data_p->write_flag == 1) {
		mask |= POLLOUT | POLLWRNORM;
	}
	/* Release semaphore. */
	up(&(data_p->sem));

	printk(KERN_DEBUG "EXAMPLE: poll returned mask 0x%x\n", mask);

	return mask;
}

static struct file_operations example_fops = {
	.open = example_open,
	.release = example_close,
	.read = example_read,
	.write = example_write,
	.llseek = example_llseek,
	.unlocked_ioctl = example_ioctl,
	.poll = example_poll,
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
