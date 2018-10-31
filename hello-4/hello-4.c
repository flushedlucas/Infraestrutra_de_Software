#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

static int __init init_hello_4(void)
{
  printk(KERN_INFO "Hello, world 4\n");
  return 0;
}

static void __exit cleanup_hello_4(void)
{
  printk(KERN_INFO "Goodbye, world 4\n");
}

module_init(init_hello_4);
module_exit(cleanup_hello_4);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lucas Silva");
MODULE_DESCRIPTION("Aprendendo a mexer no Kernel");
MODULE_SUPPORTED_DEVICE("testdevice");
