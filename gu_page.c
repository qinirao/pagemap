#include <linux/module.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

#include <linux/init.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/errno.h>

#include <linux/pagemap.h>

#define LED_MAJOR 42
#define DEVICE_NAME "simpleuser"

/*
 * Prototypes
 */
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_write(struct file *filp, const char __user *buf, size_t count, loff_t * ppos);

static struct file_operations led_ops = {
	.owner    = THIS_MODULE,
	.open     = device_open,
	.release  = device_release,
	.write =	device_write
};

static struct class *mmap_class;

static int device_open(struct inode *inode, struct file *file)
{
	printk ("device_open: %d.%d\n", MAJOR (inode->i_rdev),
		 MINOR (inode->i_rdev));
	return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
	pr_info("device_release: %d.%d\n", MAJOR (inode->i_rdev),
		 MINOR (inode->i_rdev));
	return 0;
}

static ssize_t device_write(struct file *filp, const char __user *buf, size_t count, loff_t * ppos)
{
    int res;
    unsigned long uaddr;
    char addrstr[80];
    struct page *page;
    char *my_page_address;

    unsigned long copied = copy_from_user(addrstr, buf, sizeof(addrstr));
    if (copied != 0)
    {
        pr_err("Told to copy %d, but only copied %lu\n", count, copied);
    }
    uaddr = simple_strtoul(addrstr, NULL, 0);
    
    down_read(&current->mm->mmap_sem);
	res = get_user_pages(current, current->mm,
		                 uaddr,
		                 1, /* Only want one page */
		                 1, /* Do want to write into it */
		                 1, /* do force */
		                 &page,
		                 NULL);
    if (res == 1) {
        pr_err("Got page\n");
        /* Do something with it */
	printk("lg test 1 page %p phy=%p\n",page,page_to_phys(page));
	memset(addrstr, 0 , sizeof(addrstr));
	sprintf(addrstr,"%x",page_to_phys(page));
	copy_to_user(buf, addrstr, sizeof(addrstr));
        my_page_address = kmap(page);
	printk("lg test 2 phy=%p\n",page_to_phys(page));
        strcpy (my_page_address, "Hello, is it me you're looking for?\n");
        pr_err("Got address %p and user told me it was %lx\n",my_page_address, uaddr);
        pr_err("Wrote: %s", my_page_address);
        
        kunmap(page);
        
        /* Clean up */
    if (!PageReserved(page))
        SetPageDirty(page);
	    page_cache_release(page);
    } else {
        pr_err("Couldn't get page :(\n");
    }
	up_read(&current->mm->mmap_sem);
    
    
    return count;
}

static int __init el504_init(void)
{
	int ret ;
	
	/* Register the character device */
	ret = register_chrdev (LED_MAJOR, DEVICE_NAME, &led_ops);
	if (ret < 0) {
		pr_alert("el504_init: failed with %d\n", ret);
		return ret;
	}

	mmap_class = class_create (THIS_MODULE, "mmap");
	device_create (mmap_class, NULL, MKDEV (LED_MAJOR, 0), NULL, "mmap");
	return 0;
}

static void __exit el504_exit(void)
{
	pr_debug("Goodbye\n");
	device_destroy (mmap_class, MKDEV (LED_MAJOR, 0));
	class_destroy (mmap_class);
	unregister_chrdev (LED_MAJOR, DEVICE_NAME);
}

module_init (el504_init);
module_exit (el504_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Nick Glynn");
MODULE_DESCRIPTION ("Example for get_user_pages()");

