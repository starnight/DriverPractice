PROJ=example
obj-m := $(PROJ).o

ifeq ($(KERNELDIR),)
KERNELDIR=/lib/modules/$(shell uname -r)/build
endif

all:
	make -C $(KERNELDIR) M=$(PWD) modules

install:
	sudo insmod $(PROJ).ko

uninstall:
	sudo rmmod $(PROJ)

test:
	make install; echo
	cat /proc/kallsyms | grep $(PROJ); echo
	make uninstall

clean:
	make -C $(KERNELDIR) M=$(PWD) clean
