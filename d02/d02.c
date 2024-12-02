#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/wait.h>
#include <linux/signal.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/atomic.h>
#include <linux/irqflags.h>
#include <linux/smp.h>
#include <linux/rwlock.h>
#include <linux/sched.h>
#include <linux/fs.h>

static DECLARE_WAIT_QUEUE_HEAD(myevent_waitqueue);
static rwlock_t myevent_lock = __RW_LOCK_UNLOCKED(myevent_lock);
unsigned int myevent_id = 0; 
static struct task_struct *my_thread; 

static int run_umode_handler(unsigned int event_id)
{
	char event_string[32];
	snprintf(event_string, sizeof(event_string), "Event ID: %u", event_id);

	pr_info("User mode handler invoked: %s\n", event_string);

	return 0; 
}

static int mykthread(void *unused)
{
	unsigned int event_id = 0; 
	DECLARE_WAITQUEUE(wait, current); 

	allow_signal(SIGKILL);
	add_wait_queue(&myevent_waitqueue, &wait); 

	for(;;)
	{
		set_current_state(TASK_INTERRUPTIBLE); 
		schedule();


		if (kthread_should_stop()) break;
//if (signal_pending(current)) break; 

		read_lock(&myevent_lock);
		if(myevent_id)
		{
			event_id = myevent_id; 
			read_unlock(&myevent_lock);

			run_umode_handler(event_id);
		}
		else 
		{
			read_unlock(&myevent_lock);
		}
	}
	

	set_current_state(TASK_RUNNING);
	remove_wait_queue(&myevent_waitqueue, &wait); 
	return 0;
}

void trigger_event(unsigned int event_id_value)
{
	write_lock(&myevent_lock);
	myevent_id = event_id_value;
	wake_up(&myevent_waitqueue);
	write_unlock(&myevent_lock);
}

static int __init d02_init(void)
{
	pr_info("Loading mydrv module\n");


	myevent_id = 0; 

	trigger_event(1);
	my_thread = kthread_run(mykthread, NULL, "my_kthread");
	if(IS_ERR(my_thread))
	{
		pr_err("Failed to create kernel thread\n");
		return PTR_ERR(my_thread);
	}

		
	return 0; 
}


static void __exit d02_exit(void)
{
	pr_info("Unloading mydrv module\n");

	if(my_thread)
	{
		kthread_stop(my_thread);
	}
}

module_init(d02_init);
module_exit(d02_exit);
MODULE_AUTHOR("ZG");
MODULE_LICENSE("GPL");
