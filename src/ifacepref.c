#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

const char * const NAME="ifacepref";
const unsigned int DEV_COUNT = 1;
dev_t major = 0;

static int
ifacepref_init(void)
{
    int error;
    
    printk(KERN_ALERT "ifacepref init\n");

    /* major number allocation (dynamic or static) */
    if (!major)
        error = alloc_chrdev_region(&major, 0, DEV_COUNT, NAME); /* first minor is 0 */    
    else
        error = register_chrdev_region(major, DEV_COUNT, NAME); 
    if (error)
        return -1;

    return 0;
}

static void ifacepref_exit(void)
{
    printk(KERN_ALERT "ifacepref exit\n");

    /* free allocated device numbers */
    unregister_chrdev_region(major, DEV_COUNT); 
}

module_init(ifacepref_init);
module_exit(ifacepref_exit);
