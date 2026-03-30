#ifndef ADC_READ_H
#define ADC_READ_H

#include "esp_adc/adc_oneshot.h"
#include "led.h"

const static char *TAG = "ADC_READER";

//Initialize ADCs
void init_adc();

//ADC calibration functions
static bool example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle);
static void example_adc_calibration_deinit(adc_cali_handle_t handle);

bool check_driver_voltages();

#endif