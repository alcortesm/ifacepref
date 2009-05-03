# Comment/uncomment to disable/enable debugging
DEBUG = y

ifeq ($(DEBUG),y)
    DEBFLAGS = -O -g -DIFACEPREF_DEBUG # -0 is needed to expand inlines
else
    DEBFLAGS = -O2
endif
EXTRA_CFLAGS += $(DEBFLAGS)

.PHONY: install default clean

install: default
	cp ifacepref.ko   /tmp/
	cp ifacepref_load /tmp/
	cp ifacepref_unload /tmp/
	cp ifacepref_test /tmp/
	cp ifacepref_read /tmp/
	cp ifacepref_write /tmp/
	cp ifacepref_watch /tmp/
	mkdir -p ${HOME}/man/man4/
	cp ifacepref.4  ${HOME}/man/man4/

# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.
ifneq ($(KERNELRELEASE),)
    ifacepref-objs := main.o    
    obj-m := ifacepref.o

# Otherwise we were called directly from the command
# line; invoke the kernel build system.
else

    extdatadir := $(shell cat ../CONFIGUREME | egrep -v '^\#.*$$' | grep EXTDATADIR | cut -d'=' -f2)
    kernelversion := $(shell cat ../CONFIGUREME | egrep -v '^\#.*$$' | grep KERNELVERSION | cut -d'=' -f2)
    modulesdir := $(extdatadir)/linux-src/linux-$(kernelversion)/modules/lib/modules/$(kernelversion)/build
    KERNELDIR ?= $(modulesdir)
    PWD := $(shell pwd)

default: ifacepref_test ifacepref_watch ifacepref_read ifacepref_write
	$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=um modules

ifacepref_write: write.o
	$(CC) -Wall -g -o ifacepref_write write.o

write.o: write.c
	$(CC) -Wall -g -c write.c

ifacepref_read: read.o
	$(CC) -Wall -g -o ifacepref_read read.o

read.o: read.c
	$(CC) -Wall -g -c read.c

ifacepref_test: test.o
	$(CC) -Wall -g -o ifacepref_test test.o

test.o: test.c
	$(CC) -Wall -g -c test.c

ifacepref_watch: watch.o
	$(CC) -Wall -g -o ifacepref_watch watch.o

watch.o: watch.c
	$(CC) -Wall -g -c watch.c

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=um clean
	- rm -f modules.order
	- rm -f *.o
	- rm -f ifacepref_test ifacepref_read ifacepref_write ifacepref_watch

endif

