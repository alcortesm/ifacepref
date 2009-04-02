.PHONY: install default clean

install: default
	cp ifacepref.ko /tmp/

# If KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.
ifneq ($(KERNELRELEASE),)
    obj-m := ifacepref.o

# Otherwise we were called directly from the command
# line; invoke the kernel build system.
else

    KERNELDIR ?= $(HOME)/local/linux-src/linux-2.6.28.8/modules/lib/modules/2.6.28.8/build
    PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) ARCH=um modules

clean:
	make -C $(KERNELDIR) M=$(PWD) ARCH=um clean
	- rm -f modules.order

endif

