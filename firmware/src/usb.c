#include "usb.h"
#include "usb_desc.h"
#include <stdio.h>
#include <string.h>
#include <libopencm3/stm32/st_usbfs.h>
#include <libopencm3/stm32/crs.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#define RCC_USB_PINS_PORT (RCC_GPIOA)
#define USB_PINS_PORT (GPIOA)
#define USB_DM_PIN (GPIO11)
#define USB_DP_PIN (GPIO12)

#define RCC_LED_PORT (RCC_GPIOC)
#define LED_PINS_PORT (GPIOC)

#define UP_LED (GPIO6)
#define DOWN_LED (GPIO7)
#define LEFT_LED (GPIO8)
#define RIGHT_LED (GPIO9)

usbd_device *dev_global;
uint8_t usbd_control_buffer[USB_CTRL_BUFFER_SIZE];

// Callback function for control transfers. We will handle the commands
// received from the host in this function
static int simple_control_callback(usbd_device *usbd_dev, struct usb_setup_data *req, uint8_t **buf,
	uint16_t *len, int (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
	(void) complete;
	(void) usbd_dev;
	(void) len;
	(void) buf;

	// Buffer to hold data for loopback tests
	static uint8_t loopback_buffer[sizeof(usbd_control_buffer)];

	switch(req->bRequest) {
		case 0:		// Toggle LED
			gpio_toggle(LED_PINS_PORT, RIGHT_LED);
			return USBD_REQ_HANDLED;
			break;

		case 1:		// Loopback write
			toggle_led();
			if (req->wLength > sizeof(usbd_control_buffer)) {
				return USBD_REQ_NOTSUPP;
			}
			memcpy(loopback_buffer, *buf, req->wLength);
			return USBD_REQ_HANDLED;
			break;

		case 2:		// Loopback read
			// toggle_led();
			if (req->wLength > sizeof(usbd_control_buffer)) {
				return USBD_REQ_NOTSUPP;
			}
			*len = req->wLength;
			memcpy(*buf, loopback_buffer, *len);
			return USBD_REQ_HANDLED;
			break;
		default:
			break;
	}

	return USBD_REQ_NOTSUPP;
}

// Callback for when the OUT endpoint receives data
static void data_rx_cb(usbd_device *usbd_dev, uint8_t ep)
{
	(void)ep;

    char bulk_buf[64];
    int len = 0;
	// Read the data that was transferred to OUT endpoint
	len = usbd_ep_read_packet(usbd_dev, 0x01, bulk_buf, 64);
	// If we received non-zero amount of data, send it back using IN endpoint
	if (len) {
		while (usbd_ep_write_packet(usbd_dev, 0x81, bulk_buf, len) == 0);
	}
}

// Callback function for set configuration command
static void usb_set_config_cb(usbd_device *usbd_dev, uint16_t wValue)
{
	(void)wValue;

	// Setup OUT bulk endpoint
	usbd_ep_setup(usbd_dev, 0x01, USB_ENDPOINT_ATTR_BULK, 64, data_rx_cb);
	// Setup IN bulk endpoint
	usbd_ep_setup(usbd_dev, 0x81, USB_ENDPOINT_ATTR_BULK, 64, NULL);

	// Register our custom function for control transfers
	usbd_register_control_callback(
				usbd_dev,					// USB device
				USB_REQ_TYPE_VENDOR | USB_REQ_TYPE_INTERFACE,
				USB_REQ_TYPE_TYPE | USB_REQ_TYPE_RECIPIENT,
				simple_control_callback);	// Callback function
}

void usb_init(void)
{
    usbd_device *dev;

    // Enable USB pins port peripheral clock
    rcc_periph_clock_enable(RCC_USB_PINS_PORT);
    // Enable USB peripheral clock
    rcc_periph_clock_enable(RCC_USB);
    // Use USB SOF as correction source for HSI48
    crs_autotrim_usb_enable();
    // USE HSI48 for USB clock
    rcc_set_usbclk_source(HSI48);
    // Configure USB_DP & USB_DM pins
	gpio_mode_setup(USB_PINS_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, USB_DP_PIN | USB_DM_PIN);

    dev = usbd_init(&st_usbfs_v2_usb_driver,		    // USB driver
						&dev_desc,						// Device descriptor
						&config_desc,					// Configuration descriptor
						usb_strings,					// USB strings
						4,								// Number of USB strings
						usbd_control_buffer,			// Control buffer
						sizeof(usbd_control_buffer));	// Control buffer size

	// Register the callback function for Set Configuration command
	usbd_register_set_config_callback(dev, usb_set_config_cb);

    dev_global = dev;
}

void usb_isr(void)
{
    usbd_poll(dev_global);
}
