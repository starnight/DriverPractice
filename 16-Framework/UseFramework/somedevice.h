#ifndef __EMULATE_SOME_DEVICE_H__
#define __EMULATE_SOME_DEVICE_H__

#include <linux/device.h>

/* Emulate there is a kind of device. */
/* Some device's structure. */
struct some_device {
	void *bus;
	int address;
	struct device dev;
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
#include <string.h>
#define somedevice_write(sd, len)	(memcpy((sd)->__buf, (sd)->tx_buf, len))
#define somedevice_read(sd, len)	(memcpy((sd)->rx_buf, (sd)->__buf, len))

#endif
