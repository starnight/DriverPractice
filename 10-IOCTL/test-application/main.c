#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "../test_ioctl.h"

int main(int argc, char **argv) {
	char *path;
	char *cmdstr;
	int cmd = 0;
	int flag;
	int fd;
	char pstr[40];

	/* Parse command. */
	if(argc >= 3) {
		path = argv[1];
		if(strncmp(argv[2], "GET", 3) == 0)
			cmd = IOCTL_GET_FLAG;
		if(strncmp(argv[2], "SET", 3) == 0)
			cmd = IOCTL_SET_FLAG;
		if(cmd == IOCTL_SET_FLAG)
			sscanf(argv[3], "%d", &flag);
	}
	else {
		printf("Need more arguments.\r\n");
		return -1;
	}

	/* Open device node. */
	fd = open(path, O_RDWR);
	if(fd == -1) {
		sprintf(pstr, "Open %s failed", path);
		perror(pstr);
		return -1;
	}

	/* I/O control by each command. */
	switch(cmd) {
		case IOCTL_GET_FLAG:
			if(ioctl(fd, IOCTL_GET_FLAG, &flag) == -1) {
				sprintf(pstr, "IOCTL %s failed", path);
				perror(pstr);
				return -1;
			}
			printf("Get %s flag = %d\r\n", path, flag);
			break;
		case IOCTL_SET_FLAG:
			if(ioctl(fd, IOCTL_SET_FLAG, &flag) == -1) {
				sprintf(pstr, "IOCTL %s failed", path);
				perror(pstr);
				return -1;
			}
			printf("Set %s flag = %d\r\n", path, flag);
			break;
		default:
			printf("No IOCTL command: %s\r\n", argv[2]);
	}

	/* Close device node. */
	close(fd);

	return 0;
}
