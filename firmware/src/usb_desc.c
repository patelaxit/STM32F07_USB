#include "usb_desc.h"

// USB Device Descriptor
const struct usb_device_descriptor dev_desc = {
	.bLength = USB_DT_DEVICE_SIZE,
	.bDescriptorType = USB_DT_DEVICE,
	.bcdUSB = 0x0200,
	.bDeviceClass = USB_CLASS_VENDOR,
	.bDeviceSubClass = 0,
	.bDeviceProtocol = 0,
	.bMaxPacketSize0 = 64,
	.idVendor = 0x10C4,
	.idProduct = 0x0007,
	.bcdDevice = 0x0200,
	.iManufacturer = 1,					// Index of manufacturer string
	.iProduct = 2,						// Index of product name string
	.iSerialNumber = 3,					// Index of serial number string
	.bNumConfigurations = 1,
};

// USB Endpoint Descriptor
static const struct usb_endpoint_descriptor data_endp[] = {{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x01,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = 64,
	.bInterval = 1,
},
{
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = 0x81,
	.bmAttributes = USB_ENDPOINT_ATTR_BULK,
	.wMaxPacketSize = 64,
	.bInterval = 1,
}};

// USB Interface Descriptor
static const struct usb_interface_descriptor data_iface[] = {{
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = 0,
	.bAlternateSetting = 0,
	.bNumEndpoints = 2,
	.bInterfaceClass = USB_CLASS_VENDOR,
	.bInterfaceSubClass = 0,
	.bInterfaceProtocol = 0,
	.iInterface = 0,					// Index of interface descriptor (0 -> no string)
	.endpoint = data_endp,
}};

// USB Interfaces for the configuration
static const struct usb_interface ifaces[] = {
	{
		.num_altsetting = 1,
		.altsetting = data_iface,
	}
};

// USB Configuration Descriptor
const struct usb_config_descriptor config_desc = {
	.bLength = USB_DT_CONFIGURATION_SIZE,
	.bDescriptorType = USB_DT_CONFIGURATION,
	.wTotalLength = 0,
	.bNumInterfaces = 1,
	.bConfigurationValue = 1,
	.iConfiguration = 4,				// Index of configuration string
	.bmAttributes = 0x80,
	.bMaxPower = 0x32,
	.interface = ifaces,
};

const char serial[] = "0123456789";

// USB Strings
// Note that the indexes used to refer to these strings are not 0-based
const char *usb_strings[] = {
	"AP.Corp",
	"uScope",
	serial,
	"Config.Name",
};
