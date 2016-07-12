PROJ=example
obj-m := $(PROJ).o

ifeq ($(KERNELDIR),)
KERNELDIR=/lib/modules/$(shell uname -r)/build
endif

all:
	make -C $(KERNELDIR) M=$(PWD) modules

test:
	insmod ./$(PROJ).ko
	dmesg | tail
	mknod /dev/$(PROJ) c 60 0
	chmod 666 /dev/$(PROJ)
	echo -n "Happy!" > /dev/$(PROJ)
	dmesg | tail
	cat /dev/$(PROJ)
	dmesg | tail
	rm /dev/$(PROJ)
	rmmod $(PROJ)
	dmesg | tail

clean:
	make -C $(KERNELDIR) M=$(PWD) clean
