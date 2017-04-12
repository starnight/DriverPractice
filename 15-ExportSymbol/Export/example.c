#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");

static void hello1(void) {
	printk(KERN_DEBUG "This is hello1 from example.\n");
}
EXPORT_SYMBOL(hello1);
