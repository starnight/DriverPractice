#include <linux/init.h>
#include <linux/module.h>
#include <linux/kprobes.h>

MODULE_LICENSE("Dual BSD/GPL");

/* kprobe pre_handler: called just before the probed instruction is executed */
static int handler_pre(struct kprobe *p, struct pt_regs *regs) {
#ifdef CONFIG_X86
	printk(KERN_INFO "KPROBE EXAMPLE: pre_handler: p->addr = 0x%p, ip = %lx, flags = 0x%lx\n", p->addr, regs->ip, regs->flags);
	printk(KERN_INFO "KPROBE EXAMPLE: ++++++++process_name:[%s], pid = %d.\n", current->comm, current->pid);
#endif
    dump_stack(); /* A dump_stack() here will give a stack backtrace */
    return 0;
}

/* kprobe post_handler: called after the probed instruction is executed */
static void handler_post(struct kprobe *p, struct pt_regs *regs, unsigned long flags) {
#ifdef CONFIG_X86
    printk(KERN_INFO "KPROBE EXAMPLE: post_handler: p->addr = 0x%p, flags = 0x%lx\n", p->addr, regs->flags);
#endif
}

/*
 * fault_handler: this is called if an exception is generated for any
 * instruction within the pre- or post-handler, or when Kprobes
 * single-steps the probed instruction.
*/
static int handler_fault(struct kprobe *p, struct pt_regs *regs, int trapnr) {
    printk(KERN_INFO "KPROBE EXAMPLE: fault_handler: p->addr = 0x%p, trap #%dn", p->addr, trapnr);
    /* Return 0 because we don't handle the fault. */
    return 0;
}

/* For each probe you need to allocate a kprobe structure */
static struct kprobe kp = {
	/* The kernel diver function going to be monitored.*/
	.symbol_name = "example_close",
	/* Assign the callback functions to the hooks. */
	.pre_handler = handler_pre,
	.post_handler = handler_post,
	.fault_handler = handler_fault,
};

static int kprobe_example_init(void) {
	int ret;

	printk(KERN_DEBUG "KPROBE EXAMPLE: init\n");

	/* Register the kprobe kp. */
	ret = register_kprobe(&kp);
	if(ret) {
		printk(KERN_INFO "KPROBE EXAMPLE: Failed to register a kprobe.\n");
		return ret;
	}

	printk(KERN_DEBUG "KPROBE EXAMPLE: Planted a kprobe at %p\n", kp.addr);

	return 0;
}

static void kprobe_example_exit(void) {
	printk(KERN_DEBUG "KPROBE EXAMPLE: exit\n");
	/* Unregister the allocated character device. */
	unregister_kprobe(&kp);
	printk(KERN_DEBUG "KPROBE EXAMPLE: Kprobe at %p unregistered\n", kp.addr); 
}

module_init(kprobe_example_init);
module_exit(kprobe_example_exit);
