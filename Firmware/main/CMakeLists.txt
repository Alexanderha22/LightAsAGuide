idf_component_register(SRCS "bluedroid_spp.c" "main.c"
                    PRIV_REQUIRES bt nvs_flash
                    PRIV_REQUIRES esp_driver_ledc esp_driver_gpio esp_driver_gptimer esp_ringbuf
                    INCLUDE_DIRS ".")
