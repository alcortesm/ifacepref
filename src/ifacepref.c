#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");
MODULE_VERSION("0.2");

const char * const NAME="ifacepref";
const unsigned int DEV_COUNT = 1;
const unsigned int MAJOR = 0; /* 0 means dynamic allocation */
const unsigned int MINOR = 0;
dev_t dev;

static int
ifacepref_init(void)
{
    int error;
    unsigned int major;
    
    printk(KERN_ALERT "IFACEPREF ifacepref_init() entering\n");

    /* major number allocation */
    major = MAJOR;
    if (!major)
        error = alloc_chrdev_region(&dev, MINOR, DEV_COUNT, NAME);
    else {
        dev = MKDEV(major, MINOR);
        error = register_chrdev_region(dev, DEV_COUNT, NAME);
    }
    if (error)
        return -1;
    printk(KERN_ALERT "IFACEPREF ifacepref_init() registered at major=%ud and minor=%ud\n",
            MAJOR(dev), MINOR(dev));

    printk(KERN_ALERT "IFACEPREF ifacepref_init() leaving\n");
    return 0;
}

static void ifacepref_exit(void)
{
    printk(KERN_ALERT "IFACEPREF ifacepref_exit() entering\n");

    /* free allocated device numbers */
    unregister_chrdev_region(dev, DEV_COUNT); 
    printk(KERN_ALERT "IFACEPREF ifacepref_exit() unregister major=%ud and minor=%ud\n",
            MAJOR(dev), MINOR(dev));

    printk(KERN_ALERT "IFACEPREF ifacepref_exit() leaving\n");
}

module_init(ifacepref_init);
module_exit(ifacepref_exit);
