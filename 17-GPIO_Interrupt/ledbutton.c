/*-
 * This refers to http://blog.ittraining.com.tw/2015/05/raspberry-pi-b-pi2-linux-gpio-button.html
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/of_device.h>

#define __DRIVER_NAME	"led_button"

#define	LED		27
#define	BUTTON		23

static int led = LED;
static int button = BUTTON;
static short int button_irq;
static short int light;
static unsigned long flags;

static irqreturn_t button_isr(int irq, void *data)
{
	pr_debug("%s: BUTTON is pressed", __DRIVER_NAME);

	local_irq_save(flags);
	/* Toggle the LED light */
	light = (light) ? 0 : 1;
	gpio_set_value(led, light);
	local_irq_restore(flags);

	return IRQ_HANDLED;
}

int add_led(void)
{
	int err;

	pr_debug("%s: add an LED", __DRIVER_NAME);

	err = gpio_is_valid(led);
	if (err) {
		pr_err("%s: GPIO of LED is invalid", __DRIVER_NAME);
		goto add_led_end;
	}

	err = gpio_request(led, "LED");
	if (err) {
		pr_err("%s: Request the GPIO of LED failed", __DRIVER_NAME);
		goto add_led_end;
	}

	light = 0;
	gpio_direction_output(led, light);

add_led_end:
	return err;
}

int add_button(void)
{
	int err;

	pr_debug("%s: add a BUTTON with an interrupt", __DRIVER_NAME);

	err = gpio_is_valid(button);
	if (err) {
		pr_err("%s: GPIO of BUTTON is invalid", __DRIVER_NAME);
		goto add_button_end;
	}

	err = gpio_request(button, "BUTTON");
	if (err) {
		pr_err("%s: Request the GPIO of BUTTON failed", __DRIVER_NAME);
		goto add_button_end;
	}

	button_irq = gpio_to_irq(button);
	if (button_irq < 0) {
		err = button_irq;
		pr_err("%s: Get IRQ no. from the BUTTON failed", __DRIVER_NAME);
		goto add_button_end;
	}

	err = request_irq(button_irq,
			  button_isr,
			  IRQF_TRIGGER_RISING,
			  __DRIVER_NAME,
			  NULL);
	if (err)
		pr_err("%s: Request IRQ of BUTTON failed", __DRIVER_NAME);

add_button_end:
	return err;
}

int remove_led(void)
{
	gpio_set_value(led, 0);
	gpio_free(led);

	return 0;
}

int remove_button(void)
{
	free_irq(button_irq, NULL);
	gpio_free(button);
	return 0;
}

static int ledbutton_init(void)
{
	int err;

	err = add_led();
	if (err)
		goto ledbutton_init_err;

	err = add_button();
	if (err)
		goto ledbutton_init_err;

	pr_info("%s: add the %s module", __DRIVER_NAME, __DRIVER_NAME);

	return 0;

ledbutton_init_err:
	pr_err("%s: add the %s module failed %d",
	       __DRIVER_NAME, __DRIVER_NAME, err);
	return err;
}

static void ledbutton_exit(void)
{
	remove_led();
	remove_button();
}

module_init(ledbutton_init);
module_exit(ledbutton_exit);

MODULE_AUTHOR("Jian-Hong Pan, <starnight@g.ncu.edu.tw>");
MODULE_DESCRIPTION("LED and BUTTON with an Interrupt driver");
MODULE_LICENSE("Dual BSD/GPL");
