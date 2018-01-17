#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/adc.h>
#include <stdio.h>
#include <string.h>

#include "adc.h"
#include "helper.h"

int usb_vendor_request_adc_read_vref(usbd_device *usbd_dev,
                                    struct usb_setup_data *req,
                                    uint8_t **buf,
                                    uint16_t *len,
                                    usbd_control_complete_callback *complete)
{
    // Start ADC conversion
    adc_start_conversion_regular(ADC);
    // Wait for conversion to finish
    while(!adc_eoc(ADC));
    // Read ADC value
    uint16_t adc_val = adc_read_regular(ADC);
    
    // Ensure that request data size is larger than or equal to what we're sending
    if (req->wLength <= sizeof(adc_val))
    {
        return USBD_REQ_NOTSUPP;
    }

    // Send the data out
    *len = sizeof(adc_val);
    memcpy(*buf, &adc_val, *len);
    return USBD_REQ_HANDLED;
}

void adc_init(void)
{
    // Enable ADC peripheral clock
    rcc_periph_clock_enable(RCC_ADC);

    // Power off the ADC before configuration
    adc_power_off(ADC);

    // Set clock source to the internal ADC clock
    adc_set_clk_source(ADC, ADC_CLKSOURCE_ADC);
    // Set ADC data register alignment
    adc_set_right_aligned(ADC);
    // Disable external hardware trigger. Now conversions are triggered by SW
    adc_disable_external_trigger_regular(ADC);
    // Set sample time on all channels
    adc_set_sample_time_on_all_channels(ADC, ADC_SMPTIME_071DOT5);
    // Set ADC resolution
    adc_set_resolution(ADC, ADC_RESOLUTION_12BIT);
    // Set operation mode to scan
    adc_set_operation_mode(ADC, ADC_MODE_SCAN);

    // Enable VREF sensor
    adc_enable_vref_sensor();

    // Set the channels to be read
    uint8_t channel_seq[] = {ADC_CHANNEL_VREF};
    adc_set_regular_sequence(ADC, ARRAY_LEN(channel_seq), channel_seq);

    // Calibrate ADC
    adc_calibrate_start(ADC);
    adc_calibrate_wait_finish(ADC);

    // Turn on the ADC
    adc_power_on(ADC);
    // Wait until the ADC's ready
    while(!(ADC_ISR(ADC) & ADC_ISR_ADRDY));
}