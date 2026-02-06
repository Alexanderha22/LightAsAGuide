#include "parse_data_task.h"

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/stream_buffer.h"
#include "freertos/ringbuf.h"
#include "freertos/queue.h"
#include "esp_spp_api.h"

#include "led.h"

#define PARSE_TAG "PARSER"


const TickType_t block_time = pdMS_TO_TICKS( 1000 );

void data_task_handler(void *args);
Sequence parse_data(char* sequence, size_t len);
void print_sequence(Sequence seq);

void parse_data_task(void *arg) {
    RingbufHandle_t rb = (RingbufHandle_t) arg;
    static char cmd_buf[128];
    static size_t idx = 0;

    while(1) {
        size_t size;
        char *data = (char *)xRingbufferReceive(rb, &size, block_time);
        
        if(data) {
            // parse
            ESP_LOGI(PARSE_TAG, "Chunk %d bytes", size);
            ESP_LOG_BUFFER_HEX(PARSE_TAG, data, size);

            for (size_t i = 0; i < size; i++) {
            uint8_t c = data[i];

            if (c == '\0') {
                    if (idx > 0) {
                        cmd_buf[idx] = '\0';
                        ESP_LOGI(PARSE_TAG, "CMD: %s", cmd_buf);
                        idx = 0;
                    }
                } else if (idx < sizeof(cmd_buf) - 1) {
                    cmd_buf[idx++] = c;
                }
            }

            // send command to appropriate task
            // translate led
            // run led sequence
            // 

            vRingbufferReturnItem(rb, (void *)data);
        }
    }
}