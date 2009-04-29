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
MODULE_VERSION("0.7");

struct ifacepref_dev dev;

struct file_operations ifacepref_fops = {
    .owner = THIS_MODULE,
    .read  = ifacepref_read,
    .write = ifacepref_write,
    .poll  = ifacepref_poll,
    .open  = ifacepref_open,
    .release = ifacepref_release,
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
    dev.oil.read_since_last_write = 0; /* not used, this is just the head */
    dev.oil.next = NULL; /* empty list */
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
    if (dev.oil.next)
        PDEBUG("memory lost: there were per open info for some files\n");

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
    struct ifacepref_per_open_node * nodep = (struct ifacepref_per_open_node *) filp->private_data;

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
    nodep->read_since_last_write = 1;
    up(&dev.sem);
    return ecount;
}

ssize_t
ifacepref_write(struct file * filp, const char __user *user_buff, size_t count, loff_t *offp)
{
    struct ifacepref_per_open_node * nodep;
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

    /* notify the write to all open info nodes */
    for (nodep = dev.oil.next; nodep != NULL; nodep = nodep->next)
        nodep->read_since_last_write = 0;

    wake_up_interruptible(&dev.newdataq);
    up(&dev.sem);
    return count;
}

static unsigned int
ifacepref_poll(struct file *filp, poll_table *wait)
{
    struct ifacepref_per_open_node * nodep = (struct ifacepref_per_open_node *)
        filp->private_data;
    unsigned int mask = 0 | POLLOUT | POLLWRNORM; /* ifacepref is always writable */

    down(&dev.sem);
    poll_wait(filp, &dev.newdataq, wait);
    if (!nodep->read_since_last_write) /* new data */
        mask |= POLLIN | POLLRDNORM ; /* readable */
    up(&dev.sem);
    return mask;
}

int
ifacepref_open(struct inode *inode, struct file * filp)
{
    struct ifacepref_per_open_node * nodep;
    nodep = (struct ifacepref_per_open_node *)
        kmalloc(sizeof(struct ifacepref_per_open_node), GFP_KERNEL);
    if (!nodep)
        return -ENOMEM;

    down(&dev.sem);
    nodep->next = dev.oil.next;
    dev.oil.next = nodep;
    filp->private_data = nodep;
    nodep->read_since_last_write = 0;
    up(&dev.sem);
    
    return 0;
}

int
ifacepref_release(struct inode *inode, struct file * filp)
{
    /* find current node in lists of opens and delete it */
    struct ifacepref_per_open_node * nodep;
    struct ifacepref_per_open_node * lastp;
    for (nodep = dev.oil.next, lastp = &dev.oil ;
            nodep != NULL;
            lastp = nodep, nodep = nodep->next) {
        if (nodep == filp->private_data) {
            lastp->next = nodep->next;
            kfree(nodep);
            return 0;
        }
    }
    printk(KERN_ERR "ifacepref: release: node not found on list of opens\n");
    return -EIO;
}

module_init(ifacepref_init);
module_exit(ifacepref_exit);
