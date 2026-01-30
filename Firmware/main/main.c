#include "bluedroid_spp.h"
#include "led.h"
#include "parse_data_task.h"

void app_main(void)
{
    example_ledc_init();
    bluetooth_init();
    init_data_task_handler();
}