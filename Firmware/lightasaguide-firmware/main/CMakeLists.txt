idf_component_register(SRCS "main.c"
                    PRIV_REQUIRES bt nvs_flash
                    PRIV_REQUIRES esp_driver_ledc esp_driver_gpio
                    INCLUDE_DIRS ".")
