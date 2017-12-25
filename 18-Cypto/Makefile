PROJ=mycrypto
obj-m := $(PROJ).o
ccflags-y := -DDEBUG

KERNEL_LOCATION=/lib/modules/$(shell uname -r)
BUILDDIR=$(KERNEL_LOCATION)/build

all:
	make -C $(BUILDDIR) M=$(PWD) modules

install:
	sudo make -C $(BUILDDIR) M=$(PWD) modules_install
	# Rebuild the kernel module dependencies for modprobe
	sudo depmod -a
	sudo modprobe $(PROJ)

uninstall:
	sudo modprobe -r $(PROJ)
	sudo rm $(KERNEL_LOCATION)/extra/$(PROJ).ko.xz
	# Rebuild the kernel module dependencies for modprobe
	sudo depmod -a

clean:
	make -C $(BUILDDIR) M=$(PWD) clean
