#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/string.h>

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
	size_t count;
	uint8_t byte;
	char data[] = "0123456789\r\n";
	
	printk("<1>EXAMPLE: read (size=%zu)\n", size);
	for(count = 0; (count < size) && (*f_pos) < strlen(data); ++(*f_pos), ++count) {
		byte = data[*f_pos];
		if(copy_to_user(buf + count, &byte, 1) != 0) {
			break;
		}
		printk("<1>EXAMPLE: read (buf[%zu]=%02x)\n", count, (unsigned)byte);
	}

	return count;
}

static ssize_t example_write(struct file *filp, const char __user *buf, size_t size, loff_t *f_pos) {
	size_t count;
	uint8_t byte;

	printk("<1>EXAMPLE: write (size=%zu)\n", size);
	for(count = 0; count < size; ++count) {
		if(copy_from_user(&byte, buf + count, 1) != 0) {
			break;
		}
		printk("<1>EXAMPLE: write (buf[%zu]=%02x)\n", count, (unsigned)byte);
	}

	return count;
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
