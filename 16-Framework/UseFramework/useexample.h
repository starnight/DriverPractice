#ifndef __USE_EXAMPLE_H__
#define __USE_EXAMPLE_H__

#include "../Framework/example.h"

extern int example_device_add(struct example_data *);
extern int example_device_remove(struct example_data *);
extern int example_register_driver(struct example_driver *);
extern int example_unregister_driver(struct example_driver *);

#endif
