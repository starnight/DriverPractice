PPROJ=example
PROJ=useexample
obj-m := $(PROJ).o

KBUILD_EXTRA_SYMBOLS := $(PWD)/../Export/Module.symvers

ifeq ($(KERNELDIR),)
KERNELDIR=/lib/modules/$(shell uname -r)/build
endif

all:
	make -C $(KERNELDIR) M=$(PWD) modules

test:
	sudo insmod $(PROJ).ko; echo
	cat /proc/kallsyms | grep hello1; echo
	sudo rmmod $(PROJ)
	sudo dmesg | tail -n 30

clean:
	make -C $(KERNELDIR) M=$(PWD) clean
