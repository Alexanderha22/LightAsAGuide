idf_component_register(SRCS "run_leds_task.c" "led.c" "parse_data_task.c" "bluedroid_spp.c" "main.c"
                    PRIV_REQUIRES bt nvs_flash
                    PRIV_REQUIRES esp_driver_ledc esp_driver_gpio esp_driver_gptimer esp_ringbuf
                    INCLUDE_DIRS ".")
