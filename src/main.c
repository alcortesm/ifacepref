#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/if.h>
#include <linux/semaphore.h>
#include <linux/poll.h>

#include "ifacepref.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Cortes");
MODULE_VERSION("0.6");

struct ifacepref_dev dev;

struct file_operations ifacepref_fops = {
    .owner = THIS_MODULE,
    .read  = ifacepref_read,
    .write = ifacepref_write,
    .poll  = ifacepref_poll,
};

static int
ifacepref_init(void)
{
    int err;
    
    /* initialize device resources and stuff */
    memset(&dev, '\0', sizeof(struct ifacepref_dev));
    dev.content_end =  dev.buffer;
    cdev_init(&dev.cdev, &ifacepref_fops);
    dev.cdev.owner = THIS_MODULE;
    init_MUTEX(&dev.sem);

    /* device number allocation */
    if (!IFACEPREF_MAJOR) /* dynamic allocation */
        err = alloc_chrdev_region(&(dev.number), IFACEPREF_MINOR,
                IFACEPREF_DEV_COUNT, IFACEPREF_NAME);
    else { /* static allocation */
        dev.number = MKDEV(IFACEPREF_MAJOR, IFACEPREF_MINOR);
        err = register_chrdev_region(dev.number,
                IFACEPREF_DEV_COUNT, IFACEPREF_NAME);
    }
    if (err) {
        printk(KERN_WARNING "ifacepref: can't get major %d\n",
                IFACEPREF_MAJOR);
        return err;
    }
    PDEBUG("registered device number: (%u, %u), count=%u\n",
            MAJOR(dev.number), MINOR(dev.number), IFACEPREF_DEV_COUNT);

    /* char device registration */
    err = cdev_add(&dev.cdev, dev.number, IFACEPREF_DEV_COUNT);
    if (err) {
        printk(KERN_WARNING "ifacepref: can't register char device\n");
        unregister_chrdev_region(dev.number, IFACEPREF_DEV_COUNT);
        PDEBUG("unregistered device number: (%u, %u), count=%u\n",
                MAJOR(dev.number), MINOR(dev.number), IFACEPREF_DEV_COUNT);
        return -1;
    }
    PDEBUG("registered char device\n");
    
    return 0;
}

static void
ifacepref_exit(void)
{
    cdev_del(&dev.cdev);
    PDEBUG("unregistered char device\n");

    unregister_chrdev_region(dev.number, IFACEPREF_DEV_COUNT); 
    PDEBUG("unregistered device number: (%u, %u), count=%u\n",
            MAJOR(dev.number), MINOR(dev.number), IFACEPREF_DEV_COUNT);
}

ssize_t
ifacepref_read(struct file * filp, char __user *user_buff, size_t count, loff_t *offp)
{
    const char * start;
    const char * end;
    size_t read_count;
    int pending;

    /* input sanity check */
    if (*offp < 0)
        return -EINVAL;

    /* trivial invocation */
    if (count == 0)
        return 0;

    /* calculate start and end of requested read region */
    start = dev.buffer + *offp;
    end   = start + count - 1;

    if (down_interruptible(&dev.sem))
        return -ERESTARTSYS;

    /* out of bound checks on read region */
    if (start > dev.content_end) {
        up(&dev.sem);
        return 0;
    }

    if (end > dev.content_end)
        end = dev.content_end;

    read_count = end - start + 1;

    pending = copy_to_user(user_buff, (const void *)(start), read_count);
    if (pending) {
        up(&dev.sem);
        return -EFAULT;
    }

    *offp += read_count;

    up(&dev.sem);
    return read_count;
}

ssize_t
ifacepref_write(struct file * filp, const char __user *user_buff, size_t count, loff_t *offp)
{
    char * start;
    char * end;
    int pending;

   /* input sanity checks */
   if (*offp != 0)
        return -EFBIG;

   /* trivial invocation */
   if (count == 0)
       return 0;

   /* calculate write region */
   start = dev.buffer;
   end   = start + count - 1 ;

   /* sanity checks on write region */
   if (end > IFACEPREF_BUFFER_END)
        return -ENOSPC;

    if (down_interruptible(&dev.sem))
        return -ERESTARTSYS;

    dev.content_end = end;

    pending = copy_from_user(start, user_buff, count);
    if (pending) {
        up(&dev.sem);
        return -EFAULT;
    }

    *offp += count;

    up(&dev.sem);
    return count;
}

static unsigned int
ifacepref_poll(struct file *filp, poll_table *wait)
{
    return 0;
}

module_init(ifacepref_init);
module_exit(ifacepref_exit);
