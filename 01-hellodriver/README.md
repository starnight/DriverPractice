# Hello World

This the "Hello World" practice of Linux kernel module.

## Build and Clean

* Build example.ko kernel module accroding to the *Makefile*
```sh
	make
	```

* Clean
	```sh
	make clean
	```

## Insert and Remove Module

These actions may need the super user privilege

* Insert
	```sh
	insmod example.ko
	```

* Remove
	```sh
	rmmod example.ko
	```

## Print the Kernel Ring Buffer

_printk_ in the codes will write the kernel message into the kernel ring buffer

Use ``` dmesg ``` command to examine the kernel ring buffer

## Steps

1. ``` make ```
2. ``` insmod example.ko ```
3. ``` dmesg ```
4. ``` rmmod example.ko ```
5. ``` make clean ```
