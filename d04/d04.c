#include <linux/init.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/kmod.h>

static struct delayed_work my_work;

static void delayed_shutdown(struct work_struct *work)
{
	pr_info("enter %s\n", __FUNCTION__);
	char *cmd = "/sbin/shutdown";
	char *argv[] = {
		cmd,
		"-h",
		"now",
		NULL,
	};

	char *envp[] = {
		"HOME=/",
		"PATH=/sbin:/bin:/usr:/sbin:/usr/bin",
		NULL,
	};

	call_usermodehelper(cmd, argv, envp, 0);
}


static int __init my_init(void)
{	
	pr_info("enter %s\n", __FUNCTION__);
	INIT_DELAYED_WORK(&my_work,  delayed_shutdown);

    schedule_delayed_work(&my_work, msecs_to_jiffies(200));

	return 0;
}

void my_exit(void)
{	
	pr_info("enter %s\n", __FUNCTION__);
	return;
}

module_init(my_init);
module_exit(my_exit);
MODULE_AUTHOR("ZG");
MODULE_LICENSE("GPL");
