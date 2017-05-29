#include <stdio.h>
#include <string.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/stm32/st_usbfs.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/crs.h>

#define RCC_LED_PORT (RCC_GPIOC)
#define RCC_USB_PINS_PORT (RCC_GPIOA)

#define LED_PINS_PORT (GPIOC)
#define USB_PINS_PORT (GPIOA)

#define UP_LED (GPIO6)
#define DOWN_LED (GPIO7)
#define LEFT_LED (GPIO8)
#define RIGHT_LED (GPIO9)
#define USB_DM_PIN (GPIO11)
#define USB_DP_PIN (GPIO12)

#define BULK_EP_MAXPACKET 64

// USB Device Descriptor
const struct usb_device_descriptor dev = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = USB_CLASS_VENDOR,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = BULK_EP_MAXPACKET,
	.idVendor = 0x10C4,
	.idProduct = 0x0007,
	.bcdDevice = 0x0200,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,
};

// USB Endpoint Descriptor
const struct usb_endpoint_descriptor data_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x01,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = BULK_EP_MAXPACKET,
	.bInterval = 1,
},
{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x81,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = BULK_EP_MAXPACKET,
	.bInterval = 1,
}};

// USB Interface Descriptor
const struct usb_interface_descriptor data_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 1,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_VENDOR,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,
	.endpoint = data_endp,
}};

// USB Interfaces for the configuration
const struct usb_interface ifaces[] = {
	{
		.num_altsetting = 1,
		.altsetting = data_iface,
	}
};

// USB Configuration Descriptor
const struct usb_config_descriptor config = {
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = 0,
	.bNumInterfaces = 1,
	.bConfigurationValue = 1,
	.iConfiguration = 0,
	.bmAttributes = 0x80,
	.bMaxPower = 0x32,
	.interface = ifaces,
};

// USB Strings
char serial[] = "123.456";
const char *usb_strings[] = {
	"Axit Patel",
	"STM32F072 USB Test",
	serial,
};

// Buffer to be used for control requests
uint8_t usbd_control_buffer[2*BULK_EP_MAXPACKET];

void toggle_led(void)
{
	gpio_toggle(LED_PINS_PORT, UP_LED);
}

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
			toggle_led();
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

// Buffer to hold data for the bulk transfers
char bulk_buf[64];
// Length of data for bulk transfers
int len = 0;

// Callback for when the OUT endpoint receives data
static void data_rx_cb(usbd_device *usbd_dev, uint8_t ep)
{
	(void)ep;
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

// Initialize and configure clocks
static void clock_init(void)
{
	// Setup internal 48MHz clock
	rcc_clock_setup_in_hsi48_out_48mhz();
	// Enable LED port peripheral clock
	rcc_periph_clock_enable(RCC_LED_PORT);
	// Enable USB pins port peripheral clock
	rcc_periph_clock_enable(RCC_USB_PINS_PORT);
	// Enable USB peripheral clock
	rcc_periph_clock_enable(RCC_USB);
	// Use USB SOF as correction source for HSI48
	crs_autotrim_usb_enable();
	// USE HSI48 for USB clock
	rcc_set_usbclk_source(HSI48);
}

static void gpio_init(void)
{
	// Configure LED pins
	gpio_mode_setup(LED_PINS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, UP_LED | RIGHT_LED);
	// Turn off all the LEDs
	gpio_clear(LED_PINS_PORT, UP_LED | RIGHT_LED);
	// Configure USB_DP & USB_DM pins
	gpio_mode_setup(USB_PINS_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, USB_DP_PIN | USB_DM_PIN);
}

int main(void)
{
	clock_init();
	gpio_init();

	usbd_device *usbd_dev;
	usbd_dev = usbd_init(&st_usbfs_v2_usb_driver,		// USB driver
						&dev,							// Device descriptor
						&config,						// Configuration descriptor
						usb_strings,					// USB strings
						3,								// Number of USB strings
						usbd_control_buffer,			// Control buffer
						sizeof(usbd_control_buffer));	// Control buffer size
	// Register the callback function for Set Configuration command
	usbd_register_set_config_callback(usbd_dev, usb_set_config_cb);
	// Continuously poll the USB device
	while (1) {
		usbd_poll(usbd_dev);
	}
	return 0;
}
