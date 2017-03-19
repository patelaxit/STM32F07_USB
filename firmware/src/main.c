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

// USB Device Descriptor
const struct usb_device_descriptor dev = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = 0x00,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = 64,
	.idVendor = 0x10C4,
	.idProduct = 0x0007,
	.bcdDevice = 0x0200,
	.iManufacturer = 1,
	.iProduct = 2,
	.iSerialNumber = 3,
	.bNumConfigurations = 1,
};

// USB Interface Descriptor
const struct usb_interface_descriptor iface = {
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 0,
	.bInterfaceClass = 0xFF,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,
};

// USB Interfaces for the configuration
const struct usb_interface ifaces[] = {
	{
		.num_altsetting = 1,
		.altsetting = &iface,
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
const char *usb_strings[] = {
	"Axit Patel",
	"STM32F072 USB Test",
	"1001",
};

// Buffer to be used for control requests
uint8_t usbd_control_buffer[128];

// Configure GPIO pin based on gpio_func value
void pin_set(uint32_t gpio_port, uint16_t gpio_pin, uint16_t gpio_func)
{
	if (gpio_func == 1) {
		// Turn pin on
		gpio_set(gpio_port, gpio_pin);
	} else if (gpio_func == 2) {
		// Turn pin off
		gpio_clear(gpio_port, gpio_pin);
	} else if (gpio_func == 3) {
		// Toggle pin
		gpio_toggle(gpio_port, gpio_pin);
	} else {
		return;
	}
}

// Callback function for control transfers. We will handle the commands
// received from the host in this function
static int simple_control_callback(usbd_device *usbd_dev, struct usb_setup_data *req, uint8_t **buf,
		uint16_t *len, int (**complete)(usbd_device *usbd_dev, struct usb_setup_data *req))
{
	(void)buf;
	(void)len;
	(void)complete;
	(void)usbd_dev;

	// If request type is not vendor, ignore request
	if (req->bmRequestType != 0x40)
	{
		return 0;
	}

	// Perform appropriate action based on bRequest & wValue values received from host
	// bRequest: LED: 0=UP, 1=DOWN, 2=LEFT, 3=RIGHT
	// wValue: Function: 1=ON, 2=OFF, 3=TOGGLE
	switch(req->bRequest) {
		case 0:
			pin_set(LED_PINS_PORT, UP_LED, req->wValue);
			break;
		case 1:
			pin_set(LED_PINS_PORT, DOWN_LED, req->wValue);
			break;
		case 2:
			pin_set(LED_PINS_PORT, LEFT_LED, req->wValue);
			break;
		case 3:
			pin_set(LED_PINS_PORT, RIGHT_LED, req->wValue);
			break;
		default:
			break;
	}
	return 1;
}

// Callback function for set configuration command
static void usb_set_config_cb(usbd_device *usbd_dev, uint16_t wValue)
{
	(void)wValue;
	// Register our custom function for control transfers
	usbd_register_control_callback(
				usbd_dev,					// USB device
				USB_REQ_TYPE_VENDOR,		// Type: Vendor
				USB_REQ_TYPE_TYPE,			// Type Mask: Request type
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
	gpio_mode_setup(LED_PINS_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, UP_LED |
																DOWN_LED |
																LEFT_LED |
																RIGHT_LED);
	// Turn off all the LEDs
	gpio_clear(LED_PINS_PORT, UP_LED | DOWN_LED | LEFT_LED | RIGHT_LED);
	// Configure USB_DP & USB_DM pins
	gpio_mode_setup(USB_PINS_PORT, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, USB_DP_PIN | USB_DM_PIN);
}

int main(void)
{
	clock_init();
	gpio_init();

	// Setup USB
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
