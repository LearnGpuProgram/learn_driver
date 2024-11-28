#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/kthread.h>

struct mydrv_work_queue
{
	spinlock_t lock;
	struct list_head mydrv_worklist;
	wait_queue_head_t todo;
};

struct mydrv_work
{
	struct list_head mydrv_workitem;
	void (*worker_func)(void*);
	void *worker_data;
};

static struct mydrv_work_queue mydrv_wq;

void work_func1(void *data)
{
	pr_info("Work function 1 is processing data%d \n", *(int*)data);
	msleep(1000);
}

void work_func2(void *data)
{
	pr_info("Work function 2 is processing data%d \n", *(int*)data);
	msleep(1000);
}

static void submit_work(void (*func)(void*), void *data)
{
	struct mydrv_work *new_work;

	new_work = kmalloc(sizeof(struct mydrv_work), GFP_KERNEL);
	if (!new_work)
	{
		pr_err("Failed to allocate memory for new work\n");
		return;
	}

	new_work->worker_func = func;
	new_work->worker_data = data;

	spin_lock(&mydrv_wq.lock);
	list_add_tail(&new_work->mydrv_workitem, &mydrv_wq.mydrv_worklist);
	spin_unlock(&mydrv_wq.lock);

	wake_up_interruptible(&mydrv_wq.todo);
}

static int mydrv_worker(void *unused)
{
	DECLARE_WAITQUEUE(wait, current);

	struct mydrv_work *mydrv_work; 
	void (*worker_func)(void *);
	void *worker_data; 

	set_current_state(TASK_INTERRUPTIBLE);

	while(!kthread_should_stop())
	{
		add_wait_queue(&mydrv_wq.todo, &wait);

		if (list_empty(&mydrv_wq.mydrv_worklist))
		{
			schedule();
		}
		else 
		{
			set_current_state(TASK_RUNNING);
		}

		remove_wait_queue(&mydrv_wq.todo, &wait);

		spin_lock(&mydrv_wq.lock);

		while(!list_empty(&mydrv_wq.mydrv_worklist))
		{
			mydrv_work = list_entry(mydrv_wq.mydrv_worklist.next, struct mydrv_work, mydrv_workitem);

			worker_func = mydrv_work->worker_func;
			worker_data = mydrv_work->worker_data; 

			list_del(&mydrv_work->mydrv_workitem);
			kfree(mydrv_work);

			worker_func(worker_data);
		}

		spin_unlock(&mydrv_wq.lock); 
		set_current_state(TASK_INTERRUPTIBLE);
	}

	set_current_state(TASK_RUNNING);

	return 0;

}

static int __init d01_init(void)
{
	int *data1, *data2;

	pr_info("Loading mydrv module\n");

	spin_lock_init(&mydrv_wq.lock);
	INIT_LIST_HEAD(&mydrv_wq.mydrv_worklist);
	init_waitqueue_head(&mydrv_wq.todo);

	data1 = kmalloc(sizeof(int), GFP_KERNEL);
	data2 = kmalloc(sizeof(int), GFP_KERNEL);

	*data1 = 42; 
	*data2 = 100; 

	submit_work(work_func1, data1);
	submit_work(work_func2, data2);

	kthread_run(mydrv_worker, NULL, "mydrv_worker");
	
	return 0; 
}


static void __exit d01_exit(void)
{
	pr_info("Unloading mydrv module\n");
}

module_init(d01_init);
module_exit(d01_exit);
MODULE_AUTHOR("ZG");
MODULE_LICENSE("GPL");
