PROJ=kprobe-example-close
obj-m := $(PROJ).o

ifeq ($(KERNELDIR),)
KERNELDIR=/lib/modules/$(shell uname -r)/build
endif

MAJORSCRIPT=`dmesg | grep "EXAMPLE: $(PROJ) driver(major [0-9]\+) installed" | tail -n 1 | sed -n 's/.*major \(.*\)).*/\1/p'`

TEST_TARGET_DIR=../05-PrintkMessageLevel
TEST_TARGET_FILE=$(TEST_TARGET_DIR)/Makefile

all:
	make -C $(KERNELDIR) M=$(shell pwd) modules

test:
	#
	# -----------------Target built and insert target module-----------------
	#
	cd $(TEST_TARGET_DIR); make clean; make ; make test-pre
	#
	# -------------------------Insert Kprobe module--------------------------
	#
	cd $(PWD)
	sudo insmod ./$(PROJ).ko
	dmesg | tail
	#
	# ---------------------------Target working------------------------------
	#
	cd $(TEST_TARGET_DIR); make test-action
	#
	# ------------------------Remove target module---------------------------
	#
	cd $(TEST_TARGET_DIR); make test-post; make clean
	#
	# ------------------------Remove Kprobe module---------------------------
	#
	cd $(PWD)
	sudo rmmod $(PROJ)
	dmesg | tail

clean:
	make -C $(KERNELDIR) M=$(shell pwd) clean
