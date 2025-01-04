#define pr_fmt(fmt) "PACKT-03: " fmt 

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>

char tasklet_data[] = "We use a string; but it could be pointer to a structure";

void tasklet_function(struct tasklet_struct *t)
{
	pr_info("running %s\n", __func__);
	return;
}

DECLARE_TASKLET(my_tasklet, tasklet_function);



static int __init my_init(void)
{	
        tasklet_schedule(&my_tasklet);
        pr_info("tasklet example\n");	
	return 0;
}

void my_exit(void)
{	
	tasklet_kill(&my_tasklet);
	pr_info("tasklet example cleanup\n");
	return;
}

module_init(my_init);
module_exit(my_exit);
MODULE_AUTHOR("ZG");
MODULE_LICENSE("GPL");

