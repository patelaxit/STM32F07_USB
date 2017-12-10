#include <stdio.h>
#include <string.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include "usb.h"
#include "led.h"

// Initialize and configure clocks
static void clock_init(void)
{
	// Setup internal 48MHz clock
	rcc_clock_setup_in_hsi48_out_48mhz();
}

int main(void)
{
	clock_init();
	led_init();
	usb_init();

	// Continuously poll the USB device
	while (1)
	{
		usb_isr();
	}
	return 0;
}
