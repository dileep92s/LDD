ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
KERN_DIR=/home/ds/Code/PI2/linux
HOST_KERN_DIR=/lib/modules/$(shell uname -r)/build/

obj-m := pcd_driver.o

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) modules

clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) clean

host:
	make -C $(HOST_KERN_DIR) M=$(PWD) modules