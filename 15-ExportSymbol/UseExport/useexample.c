#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");

extern void hello1(void);

static void hello2(void) {
	hello1();
	printk(KERN_DEBUG "This is hello - 2\n");
}

static int example_init(void) {
	printk(KERN_DEBUG "USEEXAMPLE: init\n");
	hello2();
	return 0;
}

static void example_exit(void) {
	printk(KERN_DEBUG "USEEXAMPLE: exit\n");
}

module_init(example_init);
module_exit(example_exit);
