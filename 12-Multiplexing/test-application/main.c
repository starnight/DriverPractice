#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

int main(int argc, char **argv) {
	char *path;
	int fd;
	fd_set read_fds;
	struct timeval tv = {.tv_sec = 5, .tv_usec = 0};
	int ret;
	ssize_t sz;
	char pstr[40];
	char buf[16];

	/* Parse command. */
	if(argc >= 2) {
		path = argv[1];
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

	/* I/O multiplexing. */
	do{
		FD_ZERO(&read_fds);
		FD_SET(fd, &read_fds);
		ret = select(fd+1, &read_fds, NULL, NULL, &tv);
		if(ret == -1) {
			perror("Select failed");
			break;
		}
	} while(ret == 0);

	/* Read from the file descriptor. */
	if(FD_ISSET(fd, &read_fds)) {
		memset(buf, '\0', 16);
		sz = read(fd, buf, 15);
		printf("Read %d bytes: %s\r\n", sz, buf);
	}

	/* Close device node. */
	close(fd);

	return 0;
}
