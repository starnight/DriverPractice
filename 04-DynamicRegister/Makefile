PROJ=example
obj-m := $(PROJ).o

ifeq ($(KERNELDIR),)
KERNELDIR=/lib/modules/$(shell uname -r)/build
endif

MAJORSCRIPT=`dmesg | grep "EXAMPLE: $(PROJ) driver(major [0-9]\+) installed" | tail -n 1 | sed -n 's/.*major \(.*\)).*/\1/p'`

all:
	make -C $(KERNELDIR) M=$(PWD) modules

test:
	insmod ./$(PROJ).ko
	dmesg | tail
	mknod /dev/$(PROJ) c $(MAJORSCRIPT) 0
	chmod 666 /dev/$(PROJ)
	ls -l /dev/$(PROJ)
	echo -n "Happy!" > /dev/$(PROJ)
	dmesg | tail
	cat /dev/$(PROJ)
	dmesg | tail
	rm /dev/$(PROJ)
	rmmod $(PROJ)
	dmesg | tail

clean:
	make -C $(KERNELDIR) M=$(PWD) clean
