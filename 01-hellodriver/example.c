#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");

static int example_init(void) {
	printk("<1>EXAMPLE: init\n");
	return 0;
}

static void example_exit(void) {
	printk("<1>EXAMPLE: exit\n");
}

module_init(example_init);
module_exit(example_exit);
