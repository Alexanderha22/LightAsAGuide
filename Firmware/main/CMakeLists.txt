idf_component_register(SRCS "adc_read.c" "run_leds_task.c" "led.c" "parse_data_task.c" "bluedroid_spp.c" "main.c"
                    PRIV_REQUIRES bt nvs_flash
                    PRIV_REQUIRES esp_driver_ledc esp_driver_gpio esp_driver_gptimer esp_ringbuf esp_adc
                    INCLUDE_DIRS ".")
