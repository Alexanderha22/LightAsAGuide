#include "bluedroid_spp.h"
#include "led.h"

// debug (print)
void print_sequence(void) {
    ESP_LOGI(SPP_TAG, "Command: %s", StoredSequence.Command);
    ESP_LOGI(SPP_TAG, "M: %i", StoredSequence.M);
    ESP_LOGI(SPP_TAG, "N: %i", StoredSequence.N);

    for (int i = 0; i < StoredSequence.M; i++)
    {
        ESP_LOGI(SPP_TAG, "Begin Block %i: ", i);
        for (int j = 0; j < StoredSequence.N; j++)
        {
            ESP_LOGI(SPP_TAG, "Light %i: ", j);
            ESP_LOGI(SPP_TAG, "Brightness: %f", StoredSequence.blocks[i].settings[j].DutyCycle);
            ESP_LOGI(SPP_TAG, "Frequency: %f", StoredSequence.blocks[i].settings[j].Frequency);
        }
        ESP_LOGI(SPP_TAG, "End Block %i: ", i);
    }
}

void app_main(void)
{
    example_ledc_init();
    bluetooth_init();
    init_data_task_handler();
}