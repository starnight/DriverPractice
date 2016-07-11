#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

MODULE_LICENSE("Dual BSD/GPL");

static int example_open(struct inode *inode, struct file *filp) {
	printk("<1>EXAMPLE: open\n");
	return 0;
}

static int example_close(struct inode *inode, struct file *filp) {
	printk("<1>EXAMPLE: close\n");
	return 0;
}

static ssize_t example_read(struct file *filp, char __user *buf, size_t size, loff_t *f_pos) {
	size_t pos;
	uint8_t byte;
	char data[] = "0123456789\r\n";
	
	printk("<1>EXAMPLE: read (size=%zu)\n", size);
	for(pos = 0; (pos < size) && pos < 12; ++pos) {
		byte = data[pos];
		if(copy_to_user(buf + pos, &byte, 1) != 0) {
			break;
		}
		printk("<1>EXAMPLE: read (buf[%zu]=%02x)\n", pos, (unsigned)byte);
	}

	return pos;
}

static ssize_t example_write(struct file *filp, const char __user *buf, size_t size, loff_t *f_pos) {
	size_t pos;
	uint8_t byte;

	printk("<1>EXAMPLE: write (size=%zu)\n", size);
	for(pos = 0; pos < size; ++pos) {
		if(copy_from_user(&byte, buf + pos, 1) != 0) {
			break;
		}
		printk("<1>EXAMPLE: write (buf[%zu]=%02x)\n", pos, (unsigned)byte);
	}

	return size;
}

static struct file_operations example_fops = {
	.open = example_open,
	.release = example_close,
	.read = example_read,
	.write = example_write,
};

#define EXAMPLE_MAJOR	60
#define EXAMPLE_NAME	"example"

static int example_init(void) {
	int result;

	printk("<1>EXAMPLE: init\n");

	/* Register character device. */
	result = register_chrdev(EXAMPLE_MAJOR, EXAMPLE_NAME, &example_fops);
	if(result < 0) {
		printk("<1>EXAMPLE: Failed to register a character device\n");
		return result;
	}

	return 0;
}

static void example_exit(void) {
	printk("<1>EXAMPLE: exit\n");

	/* Unregister character device. */
	unregister_chrdev(EXAMPLE_MAJOR, EXAMPLE_NAME);
}

module_init(example_init);
module_exit(example_exit);
