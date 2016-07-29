#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

/* Read the device data. */
void do_read(int fd) {
	char buf[16];
	ssize_t sz;

	memset(buf, '\0', 16);

	sz = read(fd, buf, 15);
	printf("Read %d bytes: %s\r\n", sz, buf);
}

/* Write data into the device. */
void do_write(int fd) {
	char buf[] = "Ker";
	ssize_t sz;
	off_t ofs;

	sz = write(fd, buf, strlen(buf));
	printf("Write %d bytes: %s\r\n", sz, buf);
}

int main(int argc, char **argv) {
	char *path;
	int fd;
	fd_set read_fds, write_fds;
	struct timeval tv = {.tv_sec = 5, .tv_usec = 0};
	int ret;
	char pstr[40];

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
	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);
	FD_SET(fd, &read_fds);
	FD_SET(fd, &write_fds);
	do{
		ret = select(fd+1, &read_fds, &write_fds, NULL, &tv);
		if(ret == -1) {
			perror("Select failed");
			break;
		}
		printf("Select return %d\t", ret);
	} while(ret == 0);
	printf("\r\n");

	/* Read from the file descriptor if it is ready to be read. */
	if(FD_ISSET(fd, &read_fds)) {
		do_read(fd);
	}
	/* Write to the file descriptor if it is ready to be written. */
	if(FD_ISSET(fd, &write_fds)) {
		do_write(fd);
	}

	/* Close device node. */
	close(fd);

	return 0;
}
