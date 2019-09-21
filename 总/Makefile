ifneq ($(KERNELRELEASE),)
	obj-m := handlemsg.o maptable_1.o addrswap.o
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	$(MAKE) -Wall -C $(KERNELDIR) M=$(PWD) modules
clean:
	rm -f *.o *.ko *.mod.c *.order *.symvers
endif
