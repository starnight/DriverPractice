PROJ=example
obj-m := $(PROJ).o

ifeq ($(KERNELDIR),)
KERNELDIR=/lib/modules/$(shell uname -r)/build
endif

all:
	make -C $(KERNELDIR) M=$(shell pwd) modules

test-pre:
	sudo insmod ./$(PROJ).ko
	sudo dmesg | tail
	cat /sys/class/$(PROJ)/$(PROJ)/dev
	cat /sys/class/$(PROJ)/$(PROJ)/uevent
	sudo chmod 666 /dev/$(PROJ)
	ls -l /dev/$(PROJ)
	cc test-application/main.c -o ioctl

test-action:
	cat /dev/$(PROJ)
	sudo dmesg | tail -n 40
	echo Happy! > /dev/$(PROJ)
	sudo dmesg | tail -n 40
	./ioctl /dev/$(PROJ) GET
	./ioctl /dev/$(PROJ) SET 2
	./ioctl /dev/$(PROJ) GET
	sudo dmesg | tail
	cat /dev/$(PROJ)
	sudo dmesg | tail -n 40
	#echo GoGoGoGoGoGo! > /dev/$(PROJ)
	#sudo dmesg | tail -n 40
	#cat /dev/$(PROJ)
	#sudo dmesg | tail -n 40

test-post:
	sudo rmmod $(PROJ)
	sudo dmesg | tail

test: test-pre test-action test-post

clean:
	make -C $(KERNELDIR) M=$(shell pwd) clean
	rm -f ioctl
