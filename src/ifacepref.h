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
#ifndef IFACEPREF_SIZE
#define IFACEPREF_SIZE IFNAMSIZ
#endif

/* Per open info. Pointed by filp->private_data.
   Some operations like write also need to modify every
   per open info, so a linked list of every one is stored
   by a linked list by the global device data */
struct ifacepref_per_open_info {
    int read_since_last_write;
    struct ifacepref_per_open_info * next;
};

/* global device data */
struct ifacepref_dev {
    char buffer[IFACEPREF_SIZE]; /* data storage */
    char * content_end;          /* pointer to data last char */
    dev_t  number;               /* device number */
    struct cdev cdev;            /* char device */
    struct semaphore sem;        /* semaphore for mutual exclusion on concurrent access */
    struct ifacepref_per_open_info oil; /* open info list: empty node, it's just the head */
    wait_queue_head_t newdataq;  /* to wait for new data to be read */
};


/* prototypes of shared functions */
ssize_t ifacepref_read(struct file * filp, char __user *buff, size_t count, loff_t *offp);
ssize_t ifacepref_write(struct file * filp, const char __user *buff, size_t count, loff_t *offp);
static unsigned int ifacepref_poll(struct file *filp, poll_table *wait);
int ifacepref_open(struct inode *inode, struct file * filp);
int ifacepref_release(struct inode *inode, struct file * filp);

#endif /* _IFACEPREF_H_ */
