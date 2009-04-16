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
	mkdir -p ${HOME}/man/man4/
	cp ifacepref.4  ${HOME}/man/man4/

# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.
ifneq ($(KERNELRELEASE),)
    obj-m := ifacepref.o

# Otherwise we were called directly from the command
# line; invoke the kernel build system.
else

    extdatadir := $(shell cat ../CONFIGUREME | egrep -v '^\#.*$$' | grep EXTDATADIR | cut -d'=' -f2)
    kernelversion := $(shell cat ../CONFIGUREME | egrep -v '^\#.*$$' | grep KERNELVERSION | cut -d'=' -f2)
    modulesdir := $(extdatadir)/linux-src/linux-$(kernelversion)/modules/lib/modules/$(kernelversion)/build
    KERNELDIR ?= $(modulesdir)
    PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=um modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=um clean
	- rm -f modules.order

endif

