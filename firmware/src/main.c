#include <stdio.h>
#include <string.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include "usb.h"

#define RCC_LED_PORT (RCC_GPIOC)
#define LED_PINS_PORT (GPIOC)

#define UP_LED (GPIO6)
#define DOWN_LED (GPIO7)
#define LEFT_LED (GPIO8)
#define RIGHT_LED (GPIO9)

void toggle_led(void)
{
	gpio_toggle(LED_PINS_PORT, UP_LED);
}

// Initialize and configure clocks
static void clock_init(void)
{
	// Setup internal 48MHz clock
	rcc_clock_setup_in_hsi48_out_48mhz();
	// Enable LED port peripheral clock
	rcc_periph_clock_enable(RCC_LED_PORT);
}

static void gpio_init(void)
{
	// Configure LED pins
	gpio_mode_setup(LED_PINS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, UP_LED | RIGHT_LED);
	// Turn off all the LEDs
	gpio_clear(LED_PINS_PORT, UP_LED | RIGHT_LED);

}

int main(void)
{
	clock_init();
	gpio_init();
	usb_init();

	// Continuously poll the USB device
	while (1) {
		usb_isr();
	}
	return 0;
}
