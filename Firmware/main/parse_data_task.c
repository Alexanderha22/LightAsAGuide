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
        unsigned char *data = (unsigned char *)xRingbufferReceive(rb, &size, block_time);
        
        if(data) {
            ESP_LOGW(PARSE_TAG, "data received from ring buffer: %s", data);
            // parse
            // send command to appropriate task
            // translate led
            // run led sequence
            translate_command(data);

            vRingbufferReturnItem(rb, (void *)data);
        }
    }
}