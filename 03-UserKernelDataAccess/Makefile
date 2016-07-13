PROJ=example
obj-m := $(PROJ).o

ifeq ($(KERNELDIR),)
KERNELDIR=/lib/modules/$(shell uname -r)/build
endif

all:
	make -C $(KERNELDIR) M=$(shell pwd) modules

test:
	sudo insmod ./$(PROJ).ko
	dmesg | tail
	sudo mknod /dev/$(PROJ) c 60 0
	sudo chmod 666 /dev/$(PROJ)
	echo -n "Happy!" > /dev/$(PROJ)
	dmesg | tail
	cat /dev/$(PROJ)
	dmesg | tail
	sudo rm /dev/$(PROJ)
	sudo rmmod $(PROJ)
	dmesg | tail

clean:
	make -C $(KERNELDIR) M=$(shell pwd) clean
