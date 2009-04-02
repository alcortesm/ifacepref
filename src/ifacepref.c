#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION("0.1");

static int ifacepref_init(void)
{
    printk(KERN_ALERT "ifacepref init\n");
    return 0;
}

static void ifacepref_exit(void)
{
    printk(KERN_ALERT "ifacepref exit\n");
}

module_init(ifacepref_init);
module_exit(ifacepref_exit);
