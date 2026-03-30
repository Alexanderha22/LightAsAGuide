#include "adc_read.h"

//Handles
adc_oneshot_unit_handle_t adc1_handle;
adc_cali_handle_t adc1_cali_chan5_handle = NULL;
adc_cali_handle_t adc1_cali_chan4_handle = NULL;
adc_cali_handle_t adc1_cali_chan7_handle = NULL;
adc_cali_handle_t adc1_cali_chan6_handle = NULL;
adc_cali_handle_t CALI_CHANNELS[4];

//Channels for each section from section 0 - 3
adc_channel_t ADC_CHANNELS[4] = {ADC_CHANNEL_6, ADC_CHANNEL_7, ADC_CHANNEL_4, ADC_CHANNEL_5};

void init_adc(void)
{
    //Initalize ADC1
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    //Initialize all channels
    adc_oneshot_chan_cfg_t config = 
    {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_5, &config)); //Corresponds to GPIO33, monitors driver for section 3
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_4, &config)); //Corresponds to GPIO32, monitors driver for section 2
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_7, &config)); //Corresponds to GPIO35, monitors driver for section 1
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_6, &config)); //Corresponds to GPIO34, monitors driver for section 0

    //Calibrate
    bool do_calibration1_chan5 = example_adc_calibration_init(ADC_UNIT_1, ADC_CHANNEL_5, ADC_ATTEN_DB_12, &adc1_cali_chan5_handle);
    bool do_calibration1_chan4 = example_adc_calibration_init(ADC_UNIT_1, ADC_CHANNEL_4, ADC_ATTEN_DB_12, &adc1_cali_chan4_handle);
    bool do_calibration1_chan7 = example_adc_calibration_init(ADC_UNIT_1, ADC_CHANNEL_7, ADC_ATTEN_DB_12, &adc1_cali_chan7_handle);
    bool do_calibration1_chan6 = example_adc_calibration_init(ADC_UNIT_1, ADC_CHANNEL_6, ADC_ATTEN_DB_12, &adc1_cali_chan6_handle);

    CALI_CHANNELS[0] = adc1_cali_chan6_handle;
    CALI_CHANNELS[1] = adc1_cali_chan7_handle;
    CALI_CHANNELS[2] = adc1_cali_chan4_handle;
    CALI_CHANNELS[3] = adc1_cali_chan5_handle;



}

static bool example_adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        //ESP_OK(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        //ESP_OK(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        //ESP_LOGI(TAG, "eFuse not burnt, skip software calibration");
    } else {
        //ESP_LOGI(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

static void example_adc_calibration_deinit(adc_cali_handle_t handle)
{
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    ESP_LOGI(TAG, "deregister %s calibration scheme", "Curve Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_curve_fitting(handle));

#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    //ESP_LOGI(TAG, "deregister %s calibration scheme", "Line Fitting");
    ESP_ERROR_CHECK(adc_cali_delete_scheme_line_fitting(handle));
#endif
}


bool check_driver_voltages()
{
    int adc_raw[10];
    int voltage[10];
    for (int i = 0; i < 4; i++)
    {
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNELS[i], &adc_raw[0]));
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(CALI_CHANNELS[i], adc_raw[0], &voltage[0]));
        //printf("ADC%d Channel[%d] Cali Voltage: %d mV\n", ADC_UNIT_1 + 1, i, voltage[0]);

        //Check if relay needs to be cut off
        if ((voltage[0] > 2000) && (get_current_time() - get_last_relay_on_time() > 0.5))
        {
            printf("(T = %f) ERROR: HIGH DRIVER VOLTAGE (%d mV)\n", get_current_time(), voltage[0]);
            turn_off_relay();
            return false;
        }
        else if ((voltage[0] < 1500) && (relayState == ON) && (get_current_time() - get_last_relay_off_time() > 0.5)) //Low voltage when leds supposed to be on
        {
            printf("ERROR: LOW DRIVER VOLTAGE (%d mV)\n", voltage[0]);
            turn_off_relay();
            return false;
        }
    }

    return true;
}