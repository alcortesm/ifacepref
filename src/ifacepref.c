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

struct file_operations fops = {
    .owner = THIS_MODULE,
};

static int
ifacepref_init(void)
{
    int error;
    unsigned int major;
    
    printk(KERN_ALERT "IFACEPREF ifacepref_init() entering\n");

    /* major number allocation */
    major = MAJOR;
    if (!major)
        error = alloc_chrdev_region(&devnum, MINOR, DEV_COUNT, NAME);
    else {
        devnum = MKDEV(major, MINOR);
        error = register_chrdev_region(devnum, DEV_COUNT, NAME);
    }
    if (error)
        return -1;
    printk(KERN_ALERT "IFACEPREF ifacepref_init() registered at major=%u, minor=%u\n, count=%u\n",
            MAJOR(devnum), MINOR(devnum), DEV_COUNT);

    /* char device registration */
    cdevp = cdev_alloc();
    cdevp->ops = &fops;
    cdevp->owner = THIS_MODULE;
    error = cdev_add(cdevp, devnum, DEV_COUNT);
    if (error) {
        printk(KERN_ALERT "IFACEPREF ifacepref_init() error adding char device to major=%u, minor=%u, count=%u\n",
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

static void ifacepref_exit(void)
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

module_init(ifacepref_init);
module_exit(ifacepref_exit);
