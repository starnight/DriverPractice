#ifndef __EMULATE_SOME_DEVICE_H__
#define __EMULATE_SOME_DEVICE_H__

#include <linux/device.h>

#define SOME_DEVICE_BUF_LEN	256

/* Emulate there is a kind of device. */
/* Some device's structure. */
struct some_device {
	void *bus;
	int address;
	struct device dev;
	uint8_t buf[SOME_DEVICE_BUF_LEN];
	uint16_t buflen;
	uint16_t bufmaxlen;
};

/* Set device's driver data. */
static inline void somedevice_set_drvdata(struct some_device *sd, void *data) {
	dev_set_drvdata(&(sd->dev), data);
}

/* Get device's driver data. */
static inline void * somedevice_get_drvdata(struct some_device *sd) {
	return dev_get_drvdata(&(sd->dev));
}

/* These are for hardware emulating. */
#include <linux/string.h>

/* Write into some device's buffer. */
static inline ssize_t somedevice_write(struct some_device *sd, void *buf, size_t size) {
	int len;
	int res;
	ssize_t c = 0;

	while(size > 0) {
		res = sd->bufmaxlen - sd->buflen;
		if(res <= 0)
			break;
		len = (res <= size) ? res : size;
		memcpy(sd->buf + sd->buflen, buf, len);
		sd->buflen += len;
		c += len;
		size -= len;
	}

	return c;
}

/* Read from some device's buffer. */
static inline ssize_t somedevice_read(struct some_device *sd, void *buf, size_t size) {
	int len;

	len = (sd->buflen <= size) ? sd->buflen : size;
	memcpy(buf, sd->buf, len);
	sd->buflen -= len;

	return len;
}

#endif
