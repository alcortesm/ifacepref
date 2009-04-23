#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/if.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alberto Cortes");
MODULE_VERSION("0.4");

/* define PDEBUG & PDEBUGG */
#undef PDEBUG
#ifdef IFACEPREF_DEBUG
#  ifdef __KERNEL__
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "ifacepref: " fmt, ## args)
#  else
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging */
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...) /* placeholder */

const char * const NAME="ifacepref";
const unsigned int DEV_COUNT = 1;
const unsigned int MAJOR = 0; /* 0 means dynamic allocation */
const unsigned int MINOR = 0;

struct dev {
    char buffer[IFNAMSIZ];
    char * content_end;
    dev_t  num;
    struct cdev * cdevp;
} dev;
#define buffer_end (dev.buffer + IFNAMSIZ - 1)

int ifacepref_open(struct inode *inodep, struct file * filp);
int ifacepref_release(struct inode *inodep, struct file * filp);
ssize_t ifacepref_read(struct file * filp, char __user *buff, size_t count, loff_t *offp);
ssize_t ifacepref_write(struct file * filp, const char __user *buff, size_t count, loff_t *offp);
struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = ifacepref_read,
    .write = ifacepref_write,
};

static int
ifacepref_init(void)
{
    int err;
    unsigned int major;
    int i;
    
    PDEBUG("ifacepref_init() entering\n");

    PDEBUG("ifacepref_init() cleaning buffer\n");
    for (i=0; i<IFNAMSIZ; i++)
        dev.buffer[i] = '\0';
    dev.content_end =  dev.buffer;

    /* major number allocation */
    major = MAJOR;
    if (!major)
        err = alloc_chrdev_region(&(dev.num), MINOR, DEV_COUNT, NAME);
    else {
        dev.num = MKDEV(major, MINOR);
        err = register_chrdev_region(dev.num, DEV_COUNT, NAME);
    }
    if (err)
        return -1;
    PDEBUG("ifacepref_init() registered at major=%u, minor=%u, count=%u\n",
            MAJOR(dev.num), MINOR(dev.num), DEV_COUNT);

    /* char device registration */
    dev.cdevp = cdev_alloc();
    dev.cdevp->ops = &fops;
    dev.cdevp->owner = THIS_MODULE;
    err = cdev_add(dev.cdevp, dev.num, DEV_COUNT);
    if (err) {
        printk(KERN_ERR "ifacepref_init() err adding char device to major=%u, minor=%u, count=%u\n",
                MAJOR(dev.num), MINOR(dev.num), DEV_COUNT);
        unregister_chrdev_region(dev.num, DEV_COUNT);
        PDEBUG("ifacepref_init() unregister major=%u, minor=%u, count=%u\n",
                MAJOR(dev.num), MINOR(dev.num), DEV_COUNT);
        return -1;
    }
    PDEBUG("ifacepref_init() char device added to major=%u, minor=%u, count=%u\n",
            MAJOR(dev.num), MINOR(dev.num), DEV_COUNT);
    
    printk(KERN_INFO "ifacepref_init() leaving\n");
    PDEBUG("ifacepref_init() leaving\n");
    return 0;
}

static void
ifacepref_exit(void)
{
    PDEBUG("ifacepref_exit() entering\n");

    /* free allocated device numbers */
    unregister_chrdev_region(dev.num, DEV_COUNT); 
    PDEBUG("ifacepref_exit() unregister major=%u, minor=%u, count=%u\n",
            MAJOR(dev.num), MINOR(dev.num), DEV_COUNT);

    /* unregister char device */
    PDEBUG("ifacepref_exit() unregistering char device major=%u, minor=%u, count=%u...",
            MAJOR(dev.cdevp->dev), MINOR(dev.cdevp->dev), dev.cdevp->count);
    cdev_del(dev.cdevp);
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
   if (end > buffer_end) {
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
