PROJ=example
obj-m := $(PROJ).o

ifeq ($(KERNELDIR),)
KERNELDIR=/lib/modules/$(shell uname -r)/build
endif

all:
	make -C $(KERNELDIR) M=$(shell pwd) modules

test-pre:
	sudo insmod ./$(PROJ).ko
	dmesg | tail
	cat /sys/class/$(PROJ)/$(PROJ)/dev
	cat /sys/class/$(PROJ)/$(PROJ)/uevent
	sudo chmod 666 /dev/$(PROJ)
	ls -l /dev/$(PROJ)
	echo "Ker ~ Ker ~" > dummy.txt

test-action:
	cat /dev/$(PROJ)
	dmesg | tail -n 40
	echo "Happy World!" > /dev/$(PROJ)
	dmesg | tail -n 40
	cat /dev/$(PROJ)
	dmesg | tail -n 40

test-post:
	sudo rmmod $(PROJ)
	dmesg | tail -n 60
	rm dummy.txt

test: test-pre test-action test-post

clean:
	make -C $(KERNELDIR) M=$(shell pwd) clean
	rm -f select
