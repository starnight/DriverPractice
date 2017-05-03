# UserKernelDataAccess

This is the following step of "Character Device".

Implement the **read** and **write** interface between user and kernel space of the file operation.

## copy\_from\_user and copy\_to\_user

* _copy\_from\_user_: Copy a block of data from user space into kernel space. 

* _copy\_to\_user_: Copy a block of data into user space from kernel space.

* Why not access the buffer in the arguments directly?
  http://stackoverflow.com/questions/12666493/why-do-you-have-to-use-copy-to-user-copy-from-user-to-access-user-space-from

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
