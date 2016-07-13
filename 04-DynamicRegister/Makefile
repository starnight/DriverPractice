PROJ=example
obj-m := $(PROJ).o

ifeq ($(KERNELDIR),)
KERNELDIR=/lib/modules/$(shell uname -r)/build
endif

MAJORSCRIPT=`dmesg | grep "EXAMPLE: $(PROJ) driver(major [0-9]\+) installed" | tail -n 1 | sed -n 's/.*major \(.*\)).*/\1/p'`

all:
	make -C $(KERNELDIR) M=$(shell pwd) modules

test:
	sudo insmod ./$(PROJ).ko
	dmesg | tail
	sudo mknod /dev/$(PROJ) c $(MAJORSCRIPT) 0
	sudo chmod 666 /dev/$(PROJ)
	ls -l /dev/$(PROJ)
	echo -n "Happy!" > /dev/$(PROJ)
	dmesg | tail
	cat /dev/$(PROJ)
	dmesg | tail
	sudo rm /dev/$(PROJ)
	sudo rmmod $(PROJ)
	dmesg | tail

clean:
	make -C $(KERNELDIR) M=$(shell pwd) clean
