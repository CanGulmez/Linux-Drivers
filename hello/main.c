/* Hello World Module */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ahmet Can GULMEZ");
MODULE_DESCRIPTION("A Simple Hello World Kernel Module");

static int __init main_init(void)
{
   printk(KERN_INFO "Hello world module initialized!\n");

   return 0;
}

static void __exit main_exit(void)
{
   printk(KERN_INFO "Hello world module exited!\n");
}

module_init(main_init);
module_exit(main_exit);
