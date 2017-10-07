# GPIO & Interrupt

This is GPIO and external interrupt practice with an LED and a button.

This refers to http://blog.ittraining.com.tw/2015/05/raspberry-pi-b-pi2-linux-gpio-button.html

## Hardware

I use a Raspberry Pi, a LED, a button and some resistors to do this practice.

* GPIO_23 of Raspberry Pi: Controls the light of the LED
* GPIO_24 of Raspberry Pi: As an interrupt input signal from the button

If you use other pins or other boards, you can change the macroes of **LED** and **BUTTON** defined in ledbutton.c.

## Make, Install & Test

```sh
make
make install
sudo modprobe ledbutton
```

Use ```dmesg``` to check the module is installed.

Press the button, then the light of LED will be turned on/off.
