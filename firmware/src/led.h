#ifndef LED_H
#define LED_H

#include <libopencm3/usb/usbd.h>

int usb_vendor_request_led_toggle(usbd_device *usbd_dev,
                                  struct usb_setup_data *req,
                                  uint8_t **buf,
                                  uint16_t *len,
                                  usbd_control_complete_callback *complete);

int usb_vendor_request_led_on(usbd_device *usbd_dev,
                              struct usb_setup_data *req,
                              uint8_t **buf,
                              uint16_t *len,
                              usbd_control_complete_callback *complete);

int usb_vendor_request_led_off(usbd_device *usbd_dev,
                               struct usb_setup_data *req,
                               uint8_t **buf,
                               uint16_t *len,
                               usbd_control_complete_callback *complete);

void led_init(void);
void led_toggle(void);

#endif