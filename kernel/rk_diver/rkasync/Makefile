#/bin/bash
KERNELDIR := /home/ppq/rockchip/mdk3308/kernel
CURRENT_PATH := $(shell pwd)
obj-m := rkkeyasync.o
 

all:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean
