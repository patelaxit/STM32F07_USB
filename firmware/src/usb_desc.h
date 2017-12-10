#ifndef USB_DESC_H
#define USB_DESC_H

#include <libopencm3/usb/usbd.h>

extern const struct usb_device_descriptor dev_desc;
extern const struct usb_config_descriptor config_desc;
extern const char *usb_strings[];

#endif
