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
    dev.isnewdata = 0;
    init_waitqueue_head(&dev.newdataq);

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
    const char * firstp;
    const char * lastp;
    size_t ecount; /* effective read count */
    int pending;

    /* input sanity check */
    if (*offp < 0)
        return -EINVAL;

    /* trivial invocation */
    if (count == 0)
        return 0;

    /* calculate requested read region */
    firstp = dev.buffer + *offp;
    lastp  = firstp + count - 1;

    if (down_interruptible(&dev.sem))
        return -ERESTARTSYS;

    /* out of bound checks on read region */
    if (firstp > dev.content_end) {
        up(&dev.sem);
        return 0;
    }
    if (lastp > dev.content_end)
        lastp = dev.content_end;

    /* effective read count */
    ecount = lastp - firstp + 1;

    pending = copy_to_user(user_buff, (const void *)(firstp), ecount);
    if (pending) {
        up(&dev.sem);
        return -EFAULT;
    }

    *offp += ecount;
    up(&dev.sem);
    return ecount;
}

ssize_t
ifacepref_write(struct file * filp, const char __user *user_buff, size_t count, loff_t *offp)
{
    int pending;

   /* input sanity checks */
   if (*offp != 0)
        return -EFBIG;

   /* trivial invocation */
   if (count == 0)
       return 0;

   /* sanity checks on write region */
   if (count > IFACEPREF_SIZE)
        return -ENOSPC;

   if (down_interruptible(&dev.sem))
       return -ERESTARTSYS;

   dev.content_end = dev.buffer + count - 1;

   pending = copy_from_user(dev.buffer, user_buff, count);
   if (pending) {
       up(&dev.sem);
       return -EFAULT;
   }

    *offp += count;
    dev.isnewdata = 1;
    wake_up_interruptible(&dev.newdataq);
    up(&dev.sem);
    return count;
}

static unsigned int
ifacepref_poll(struct file *filp, poll_table *wait)
{
    unsigned int mask = 0 | POLLOUT | POLLWRNORM; /* ifacepref is always writable */
    down(&dev.sem);
    poll_wait(filp, &dev.newdataq, wait);
    if (dev.isnewdata) {
        mask |= POLLIN | POLLRDNORM ; /* readable */
        dev.isnewdata = 0;
    }
    up(&dev.sem);
    return mask;
}

module_init(ifacepref_init);
module_exit(ifacepref_exit);
