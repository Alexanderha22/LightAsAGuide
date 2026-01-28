#include "bluedroid_spp.h"
#include "led.h"

#define SPP_TAG "SPP_ACCEPTOR"
#define SPP_SERVER_NAME "SPP_SERVER"

static const char local_device_name[] = CONFIG_EXAMPLE_LOCAL_DEVICE_NAME;
static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;
static const bool esp_spp_enable_l2cap_ertm = true;

static struct timeval time_new, time_old;
static long data_num = 0;

static const esp_spp_sec_t sec_mask = ESP_SPP_SEC_AUTHENTICATE;
static const esp_spp_role_t role_slave = ESP_SPP_ROLE_SLAVE;

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

    xTaskCreate(worker_task, "worker", 4096, NULL, tskIDLE_PRIORITY + 1, &s_worker_handle);
}