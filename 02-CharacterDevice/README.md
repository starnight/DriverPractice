# Character Device

All of the devices could be operated like files.

So, let's start from the "Character Device" file kernel module.

## Create a Character Special File

1. Use ``` mknod /dev/example c 60 0 ``` to create the character special file mapped to the example.ko kernel module.

	https://linux.die.net/man/1/mknod

	* _/dev/example_: The name of the character file
	* _c_: Represent making a character device file
	* _60_: It is the **MAJOR** number that maps to the **MAJOR** number (**EXAMPLE_MAJOR**) of the example.ko kernel module
	* _0_: It is the **MINOR** number that maps to the **MINOR** number of the example.ko kernel module

2. Use ``` ls -l /dev/example ``` to observe the file

	```sh
	crw-rw-rw- 1 root root 60, 0 May  3 15:05 /dev/example
	```

	* _60_: **MAJOR** number
	* _0_: **MINOR** number

## MAJOR and MINOR Number

Generally, the **MAJOR** number identifies the device driver and the **MINOR** number identifies a particular device (possibly out of many) that the driver controls.

https://en.wikipedia.org/wiki/Device_file#Unix_and_Unix-like_systems

## Steps

1. ``` make ```
2. ``` insmod example.ko ```
3. ``` dmesg ```
4. ``` mknod /dev/example c 60 0 ```
5. ``` chmod 666 /dev/example ``` (for accessing from other users)
6. Do some file operations with the */dev/example*, like file open, read, write, close.
7. ``` dmesg ```
8. ``` rmmod example.ko ```
9. ``` rm /dev/example ```
10. ``` make clean ```
