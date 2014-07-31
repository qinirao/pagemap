#make -C /home/huawei/linaro/qemu/linux-linaro-lng/ ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- M=`pwd` modules
obj-m += gu_page.o
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean 

