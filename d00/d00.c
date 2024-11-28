#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int __init d00_init(void)
{
	pr_info("Hello world!\n");
	return 0; 
}


static void __exit d00_exit(void)
{
	pr_info("End of the world\n");
}

module_init(d00_init);
module_exit(d00_exit);
MODULE_AUTHOR("ZG");
MODULE_LICENSE("GPL");
