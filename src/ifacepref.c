#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/if.h>

MODULE_LICENSE("GPL");
MODULE_VERSION("0.3");

const char * const NAME="ifacepref";
const unsigned int DEV_COUNT = 1;
const unsigned int MAJOR = 0; /* 0 means dynamic allocation */
const unsigned int MINOR = 0;

char data[IFNAMSIZ];
char * const data_end = data + IFNAMSIZ - 1 ;
char * content_end;

dev_t devnum;

struct cdev * cdevp;

int ifacepref_open(struct inode *inodep, struct file * filp);
int ifacepref_release(struct inode *inodep, struct file * filp);
ssize_t ifacepref_read(struct file * filp, char __user *buff, size_t count, loff_t *offp);
ssize_t ifacepref_write(struct file * filp, const char __user *buff, size_t count, loff_t *offp);
struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = ifacepref_open,
    .release = ifacepref_release,
    .read = ifacepref_read,
    .write = ifacepref_write,
};

static int
ifacepref_init(void)
{
    int err;
    unsigned int major;
    int i;
    
    printk(KERN_ALERT "IFACEPREF ifacepref_init() entering\n");

    printk(KERN_ALERT "IFACEPREF ifacepref_open() initializing ifacepref to empty string\n");
    for (i=0; i<IFNAMSIZ; i++)
        data[i] = '\0';
    content_end =  data;

    /* major number allocation */
    major = MAJOR;
    if (!major)
        err = alloc_chrdev_region(&devnum, MINOR, DEV_COUNT, NAME);
    else {
        devnum = MKDEV(major, MINOR);
        err = register_chrdev_region(devnum, DEV_COUNT, NAME);
    }
    if (err)
        return -1;
    printk(KERN_ALERT "IFACEPREF ifacepref_init() registered at major=%u, minor=%u, count=%u\n",
            MAJOR(devnum), MINOR(devnum), DEV_COUNT);

    /* char device registration */
    cdevp = cdev_alloc();
    cdevp->ops = &fops;
    cdevp->owner = THIS_MODULE;
    err = cdev_add(cdevp, devnum, DEV_COUNT);
    if (err) {
        printk(KERN_ALERT "IFACEPREF ifacepref_init() err adding char device to major=%u, minor=%u, count=%u\n",
                MAJOR(devnum), MINOR(devnum), DEV_COUNT);
        unregister_chrdev_region(devnum, DEV_COUNT);
        printk(KERN_ALERT "IFACEPREF ifacepref_init() unregister major=%u, minor=%u, count=%u\n",
                MAJOR(devnum), MINOR(devnum), DEV_COUNT);
        return -1;
    }
    printk(KERN_ALERT "IFACEPREF ifacepref_init() char device added to major=%u, minor=%u, count=%u\n",
            MAJOR(devnum), MINOR(devnum), DEV_COUNT);
    

    printk(KERN_ALERT "IFACEPREF ifacepref_init() leaving\n");
    return 0;
}

static void
ifacepref_exit(void)
{
    printk(KERN_ALERT "IFACEPREF ifacepref_exit() entering\n");

    /* free allocated device numbers */
    unregister_chrdev_region(devnum, DEV_COUNT); 
    printk(KERN_ALERT "IFACEPREF ifacepref_exit() unregister major=%u, minor=%u, count=%u\n",
            MAJOR(devnum), MINOR(devnum), DEV_COUNT);

    /* unregister char device */
    printk(KERN_ALERT "IFACEPREF ifacepref_exit() unregistering char device major=%u, minor=%u, count=%u...",
            MAJOR(cdevp->dev), MINOR(cdevp->dev), cdevp->count);
    cdev_del(cdevp);
    printk(KERN_ALERT "done\n");

    printk(KERN_ALERT "IFACEPREF ifacepref_exit() leaving\n");
}

int
ifacepref_open(struct inode *inodep, struct file * filp)
{
    printk(KERN_ALERT "IFACEPREF ifacepref_open() entering\n");
    printk(KERN_ALERT "IFACEPREF ifacepref_open() leaving\n");
    return 0;
}

int
ifacepref_release(struct inode *inodep, struct file * filp)
{
    printk(KERN_ALERT "IFACEPREF ifacepref_release() entering\n");
    printk(KERN_ALERT "IFACEPREF ifacepref_release() leaving\n");
    return 0;
}

ssize_t
ifacepref_read(struct file * filp, char __user *user_buff, size_t count, loff_t *offp)
{
    const char * start;
    const char * end;
    size_t read_count;
    int pending;

    printk(KERN_ALERT "IFACEPREF ifacepref_read() entering count=%u, offset=%llu\n",
            count, *offp);

    /* input sanity check */
    if (*offp < 0) {
        printk(KERN_ALERT "IFACEPREF ifacepref_read() ERROR negative offset provided\n");
        return -EINVAL;
    }

    /* trivial invocation */
    if (count == 0)
        return 0;

    /* calculate start and end of requested read region */
    start = data + *offp;
    end   = start + count - 1;

    /* out of bound checks on read region */
    if (start > content_end) {
        printk(KERN_ALERT "IFACEPREF ifacepref_read() offset bigger than content size -> EOF; leaving with 0\n");
        return 0;
    }
    if (end > content_end)
        end = content_end;

    read_count = end - start + 1;

    pending = copy_to_user(user_buff, (const void *)(start), read_count);
    if (pending) {
        printk(KERN_ALERT "IFACEPREF ifacepref_read() ERROR not valid user-space pointer\n");
        return -EFAULT;
    }

    *offp += read_count;
    printk(KERN_ALERT "IFACEPREF ifacepref_read() %u bytes of data copied to user-space\n",
            read_count);

    printk(KERN_ALERT "IFACEPREF ifacepref_read() leaving with %u\n", read_count);
    return read_count;
}

ssize_t
ifacepref_write(struct file * filp, const char __user *user_buff, size_t count, loff_t *offp)
{
    char * start;
    char * end;
    size_t write_count;
    int pending;

    printk(KERN_ALERT "IFACEPREF ifacepref_write() entering count=%u, offset=%llu\n",
            count, *offp);
   
   /* input sanity checks */
   if (*offp != 0) {
        printk(KERN_ALERT "IFACEPREF ifacepref_write() ERROR requested offset is not 0\n");
        return -EFBIG;
   }

   /* trivial invocation */
   if (count == 0)
       return 0;

   /* calculate write region */
   start = data;
   end   = start + count - 1 ;

   /* sanity checks on write region */
   if (end > data_end) {
        printk(KERN_ALERT "IFACEPREF ifacepref_write() ERROR requested offset is not 0\n");
        return -ENOSPC;
   }

    write_count = end - start + 1 ;
    content_end = end;

    pending = copy_from_user(start, user_buff, write_count);
    if (pending) {
        printk(KERN_ALERT "IFACEPREF ifacepref_write() ERROR not valid user-space pointer\n");
        return -EFAULT;
    }
    printk(KERN_ALERT "IFACEPREF ifacepref_write() %u bytes of data copied from user-space\n",
            write_count);

    *offp += write_count;
    printk(KERN_ALERT "IFACEPREF ifacepref_write() leaving with %u\n",
            write_count);
    return write_count;
}

module_init(ifacepref_init);
module_exit(ifacepref_exit);
