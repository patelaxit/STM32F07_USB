#ifndef ADC_H
#define ADC_H

#include <libopencm3/usb/usbd.h>

int usb_vendor_request_adc_read_vref(usbd_device *usbd_dev,
                                    struct usb_setup_data *req,
                                    uint8_t **buf,
                                    uint16_t *len,
                                    usbd_control_complete_callback *complete);

void adc_init(void);

#endif