/*#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/slab.h>*/

struct birthday {
	int day;
	int month;
	int year;
	struct list_head list;
};

static LIST_HEAD(birthday_list);

// Load the module
int birthday_init(void)
{
	printk(KERN_INFO "Loading Module\n");
	// Create and initialize instances of birtday 
	struct birthday *person;

	// First person
	person = kmalloc(sizeof(*person), GFP_KERNEL);
	person->day = 2;
	person->month = 8;
	person->year = 2000;
	INIT_LIST_HEAD(&person->list);
	list_add_tail(&person->list, &birthday_list)
	// Second person
	person = kmalloc(sizeof(*person), GFP_KERNEL);
	person->day = 3;
	person->month = 4;
	person->year = 1998;
	INIT_LIST_HEAD(&person->list);
	list_add_tail(&person->list, &birthday_list)
	// Third person
	person = kmalloc(sizeof(*person), GFP_KERNEL);
	person->day = 4;
	person->month = 10;
	person->year = 1995;
	INIT_LIST_HEAD(&person->list);
	list_add_tail(&person->list, &birthday_list)
	// Fourth person
	person = kmalloc(sizeof(*person), GFP_KERNEL);
	person->day = 5;
	person->month = 9;
	person->year = 1945;
	INIT_LIST_HEAD(&person->list);
	list_add_tail(&person->list, &birthday_list)

	// Traverse and print the list
	list_for_each_entry(person, &birthday_list, list) {
		printk(KERN_INFO "%d/%d/%d \n", person->day, person->month, person->year);
	}

	return 0;
}

// Remove the module
void birthday_exit(void) {
	printk(KERN_INFO "Removing Module\n");
	struct birthday *ptr, *next;

	// Delete the elements from the list, return the free memory back to kernel
	list_for_each_entry_safe(ptr, next, &birthday_list, list) {
		list_del(&ptr->list);
		kfree(ptr);
	}
}

module_init(birthday_init);
module_exit(birthday_init);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("p3a");
MODULE_AUTHOR("Turku Savran");

