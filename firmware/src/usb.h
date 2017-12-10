#ifndef USB_H
#define USB_H

#include <libopencm3/usb/usbd.h>

#define USB_CTRL_BUFFER_SIZE 64

void usb_init(void);
void usb_isr(void);

// Handle for USB device
extern usbd_device *dev_global;
// Buffer to be used for control requests
extern uint8_t usbd_control_buffer[USB_CTRL_BUFFER_SIZE];

#endif
