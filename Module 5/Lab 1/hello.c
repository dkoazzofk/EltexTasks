#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("zxconserve");
MODULE_AUTHOR("dkoazzofk");
MODULE_DESCRIPTION("A Simple Hello World module");

static int __init hello_init(void){
    printk(KERN_INFO "Hello world!\n");
    return 0;
}

static void __exit hello_cleanup(void){
    printk(KERN_INFO "Cleaning up module.\n");
}

module_init(hello_init);
module_exit(hello_cleanup);