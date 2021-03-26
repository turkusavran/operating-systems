#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>

int p3b_init(void)
{
	printk(KERN_INFO "Loading Module\n");
    struct task_struct *task;

	if (pid == -1) {
		printk(KERN_INFO "pid is not provided.\n");
		return 0;
	}
	
	task = find_task_by_vpid(pid);

	if (task==NULL) {
		return 1;
	}

	printk(KERN_INFO "pid: %d\n", task->pid);
	printk(KERN_INFO "ppid: %d\n", task->parent->pid);
	printk(KERN_INFO "Executable name: %s\n", task->comm);

	struct task_struct *task_sibling;
	struct list_head *list;

	// Iterate over siblings
	list_for_each(list, &task->parent->children) {
		task_sibling = list_entry(list, struct task_struct, sibling);

		if (task_sibling->pid != pid) {
			printk(KERN_INFO "Sibling List\n");
			printk(KERN_INFO "pid: %d\n", task_sibling->pid);
			printk(KERN_INFO "Executable name: %s\n", task_sibling->comm);
		}
	}
	return 0;

	// If no task
	printk(KERN_INFO "pid is not valid.\n");
	return 0;
}

// Remove the module
void p3b_exit(void) {
	printk(KERN_INFO "Removing Module\n");
}

module_param(pid, int, 0);
module_init(p3b_init);
module_exit(p3b_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("p3b");
MODULE_AUTHOR("Turku Savran");