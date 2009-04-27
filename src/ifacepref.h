#ifndef _IFACEPREF_H_
#define _IFACEPREF_H_

/* macros to help debugging */
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


/* useful consts */
#ifndef IFACEPREF_NAME
#define IFACEPREF_NAME "ifacepref"
#endif
#ifndef IFACEPREF_DEV_COUNT
#define IFACEPREF_DEV_COUNT 1
#endif
#ifndef IFACEPREF_NAJOR
#define IFACEPREF_MAJOR 0 /* 0 means dynamic allocation */
#endif
#ifndef IFACEPREF_NINOR
#define IFACEPREF_MINOR 0
#endif


/* device resources and stuff */
struct ifacepref_dev {
    char buffer[IFNAMSIZ];
    char * content_end;
    dev_t  number;
    struct cdev cdev;
    struct semaphore sem;
    int isnewdata;
    wait_queue_head_t newdataq;
};
#define IFACEPREF_BUFFER_END (dev.buffer + IFNAMSIZ - 1)


/* prototypes of shared functions */
ssize_t ifacepref_read(struct file * filp, char __user *buff, size_t count, loff_t *offp);
ssize_t ifacepref_write(struct file * filp, const char __user *buff, size_t count, loff_t *offp);
static unsigned int ifacepref_poll(struct file *filp, poll_table *wait);

#endif /* _IFACEPREF_H_ */
