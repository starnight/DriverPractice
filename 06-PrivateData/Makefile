PROJ=example
obj-m := $(PROJ).o

ifeq ($(KERNELDIR),)
KERNELDIR=/lib/modules/$(shell uname -r)/build
endif

MAJORSCRIPT=`dmesg | grep "EXAMPLE: $(PROJ) driver(major [0-9]\+) installed" | tail -n 1 | sed -n 's/.*major \(.*\)).*/\1/p'`

all:
	make -C $(KERNELDIR) M=$(shell pwd) modules

test-pre:
	sudo insmod ./$(PROJ).ko
	sudo dmesg | tail
	sudo mknod /dev/$(PROJ) c $(MAJORSCRIPT) 0
	sudo chmod 666 /dev/$(PROJ)
	ls -l /dev/$(PROJ)

test-action:
	cat /dev/$(PROJ)
	sudo dmesg | tail -n 40
	echo Happy! > /dev/$(PROJ)
	sudo dmesg | tail -n 40
	cat /dev/$(PROJ)
	sudo dmesg | tail -n 40
	#echo GoGoGoGoGoGo! > /dev/$(PROJ)
	#dmesg | tail -n 40
	#cat /dev/$(PROJ)
	#dmesg | tail -n 40

test-post:
	sudo rm /dev/$(PROJ)
	sudo rmmod $(PROJ)
	sudo dmesg | tail

test: test-pre test-action test-post

clean:
	make -C $(KERNELDIR) M=$(shell pwd) clean
