#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>

MODULE_LICENSE("GPL");
MODULE_VERSION("0.2");

const char * const NAME="ifacepref";
const unsigned int DEV_COUNT = 1;
const unsigned int MAJOR = 0; /* 0 means dynamic allocation */
const unsigned int MINOR = 0;

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
    
    printk(KERN_ALERT "IFACEPREF ifacepref_init() entering\n");

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
    printk(KERN_ALERT "IFACEPREF ifacepref_init() registered at major=%u, minor=%u\n, count=%u\n",
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
ifacepref_read(struct file * filp, char __user *buff, size_t count, loff_t *offp)
{
    printk(KERN_ALERT "IFACEPREF ifacepref_read() entering\n");
    printk(KERN_ALERT "IFACEPREF ifacepref_read() leaving\n");
    return -1;
}

ssize_t
ifacepref_write(struct file * filp, const char __user *buff, size_t count, loff_t *offp)
{
    printk(KERN_ALERT "IFACEPREF ifacepref_write() entering\n");
    printk(KERN_ALERT "IFACEPREF ifacepref_write() leaving\n");
    return -1;
}

module_init(ifacepref_init);
module_exit(ifacepref_exit);
