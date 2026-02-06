#include "bluedroid_spp.h"
#include "parse_data_task.h"
#include "led.h"

void app_main(void)
{
     RingbufHandle_t rb =
        xRingbufferCreate(1024, RINGBUF_TYPE_BYTEBUF);
    assert(rb);

    init_leds();

    xTaskCreate(parse_data_task, "parser", 4096, rb, 5, NULL);

    bluetooth_init(rb);
}