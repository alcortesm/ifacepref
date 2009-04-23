#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/if.h>

#include "ifacepref.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Cortes");
MODULE_VERSION("0.4");

struct dev dev;

struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = ifacepref_read,
    .write = ifacepref_write,
};

static int
ifacepref_init(void)
{
    int err;
    
    /* initialize buffer */
    memset(dev.buffer, '\0', IFNAMSIZ);
    dev.content_end =  dev.buffer;

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
    PDEBUG("registered at major=%u, minor=%u, count=%u\n",
            MAJOR(dev.number), MINOR(dev.number), IFACEPREF_DEV_COUNT);

    /* char device registration */
    cdev_init(&dev.cdev, &fops);
    dev.cdev.owner = THIS_MODULE;
    err = cdev_add(&dev.cdev, dev.number, IFACEPREF_DEV_COUNT);
    if (err) {
        printk(KERN_WARNING "ifacepref: can't add char device\n");
        unregister_chrdev_region(dev.number, IFACEPREF_DEV_COUNT);
        PDEBUG("unregistered from major=%u, minor=%u, count=%u\n",
                MAJOR(dev.number), MINOR(dev.number), IFACEPREF_DEV_COUNT);
        return -1;
    }
    PDEBUG("char device added\n");
    
    return 0;
}

static void
ifacepref_exit(void)
{
    PDEBUG("ifacepref_exit() entering\n");

    /* free allocated device numbers */
    unregister_chrdev_region(dev.number, IFACEPREF_DEV_COUNT); 
    PDEBUG("ifacepref_exit() unregister major=%u, minor=%u, count=%u\n",
            MAJOR(dev.number), MINOR(dev.number), IFACEPREF_DEV_COUNT);

    /* unregister char device */
    PDEBUG("ifacepref_exit() unregistering char device major=%u, minor=%u, count=%u...",
            MAJOR(dev.cdev.dev), MINOR(dev.cdev.dev), dev.cdev.count);
    cdev_del(&dev.cdev);
    PDEBUG("done\n");

    PDEBUG("ifacepref_exit() leaving\n");
}

ssize_t
ifacepref_read(struct file * filp, char __user *user_buff, size_t count, loff_t *offp)
{
    const char * start;
    const char * end;
    size_t read_count;
    int pending;

    PDEBUG("ifacepref_read() entering offset=%llu, count=%u\n",
            *offp, count);

    /* input sanity check */
    if (*offp < 0) {
        PDEBUG("ifacepref_read() negative offset detected\n");
        PDEBUG("ifacepref_read() leaving with %d\n", -EINVAL);
        return -EINVAL;
    }

    /* trivial invocation */
    if (count == 0) {
        PDEBUG("ifacepref_read() leaving with 0\n");
        return 0;
    }

    /* calculate start and end of requested read region */
    start = dev.buffer + *offp;
    end   = start + count - 1;

    /* out of bound checks on read region */
    if (start > dev.content_end) {
        PDEBUG("ifacepref_read() offset bigger than content size\n");
        PDEBUG("ifacepref_read() leaving with 0\n");
        return 0;
    }
    if (end > dev.content_end)
        end = dev.content_end;

    read_count = end - start + 1;

    pending = copy_to_user(user_buff, (const void *)(start), read_count);
    if (pending) {
        PDEBUG("ifacepref_read() copy_to_user() detected an invalid user-space pointer\n");
        PDEBUG("ifacepref_read() leaving with %d\n", -EFAULT);
        return -EFAULT;
    }

    *offp += read_count;

    PDEBUG("ifacepref_read() leaving with %u\n", read_count);
    return read_count;
}

ssize_t
ifacepref_write(struct file * filp, const char __user *user_buff, size_t count, loff_t *offp)
{
    char * start;
    char * end;
    size_t write_count;
    int pending;

    PDEBUG("ifacepref_write() entering offset=%llu, count=%u\n",
            *offp, count);
   
   /* input sanity checks */
   if (*offp != 0) {
        PDEBUG("ifacepref_write() offset is not 0\n");
        PDEBUG("ifacepref_write() leaving with %d\n", -EFBIG);
        return -EFBIG;
   }

   /* trivial invocation */
   if (count == 0)
       return 0;

   /* calculate write region */
   start = dev.buffer;
   end   = start + count - 1 ;

   /* sanity checks on write region */
   if (end > IFACEPREF_BUFFER_END) {
        PDEBUG("ifacepref_write() asked to write beyond device range\n");
        PDEBUG("ifacepref_write() leaving with %d\n", -ENOSPC);
        return -ENOSPC;
   }

    write_count = end - start + 1 ;
    dev.content_end = end;

    pending = copy_from_user(start, user_buff, write_count);
    if (pending) {
        PDEBUG("ifacepref_write() copy_to_user() detected an invalid user-space pointer\n");
        PDEBUG("ifacepref_write() leaving with %d\n", -EFAULT);
        return -EFAULT;
    }

    *offp += write_count;
    PDEBUG("ifacepref_write() leaving with %u\n", write_count);
    return write_count;
}

module_init(ifacepref_init);
module_exit(ifacepref_exit);
