#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "led.h"

#define RCC_LED_PORT (RCC_GPIOC)
#define LED_PINS_PORT (GPIOC)

#define UP_LED (GPIO6)
#define DOWN_LED (GPIO7)
#define LEFT_LED (GPIO8)
#define RIGHT_LED (GPIO9)
#define ALL_LEDS UP_LED | RIGHT_LED | LEFT_LED | DOWN_LED

int usb_vendor_request_led_toggle(usbd_device *usbd_dev, 
                                struct usb_setup_data *req, 
                                uint8_t **buf, 
                                uint16_t *len,
                                usbd_control_complete_callback *complete)
{
    gpio_toggle(LED_PINS_PORT, LEFT_LED | RIGHT_LED);
    return USBD_REQ_HANDLED;
}

int usb_vendor_request_led_on(usbd_device *usbd_dev, 
                                struct usb_setup_data *req, 
                                uint8_t **buf, 
                                uint16_t *len,
                                usbd_control_complete_callback *complete)
{
    gpio_set(LED_PINS_PORT, ALL_LEDS);
    return USBD_REQ_HANDLED;
}

int usb_vendor_request_led_off(usbd_device *usbd_dev, 
                                struct usb_setup_data *req, 
                                uint8_t **buf, 
                                uint16_t *len,
                                usbd_control_complete_callback *complete)
{
    gpio_clear(LED_PINS_PORT, ALL_LEDS);
    return USBD_REQ_HANDLED;
}

void led_init(void) 
{
    // Enable LED port peripheral clock
	rcc_periph_clock_enable(RCC_LED_PORT);
    // Configure LED pins
	gpio_mode_setup(LED_PINS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, ALL_LEDS);
	// Turn on all the LEDs
	gpio_set(LED_PINS_PORT, ALL_LEDS);
}