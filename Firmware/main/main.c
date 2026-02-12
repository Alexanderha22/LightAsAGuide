#include "bluedroid_spp.h"
#include "parse_data_task.h"
#include "led.h"

void app_main(void)
{

/*     //TESTING
    init_leds();

    unsigned char inputsequence4[] = "GetInfo";

    translate_sequence_package(inputsequence4); */

    /* unsigned char inputsequence[] = "SetSection,0,20,20";
    
    translate_sequence_package(inputsequence);

    unsigned char inputsequence1[] = "SetSection,1,40,20";
    
    translate_sequence_package(inputsequence1);

    unsigned char inputsequence2[] = "SetSection,2,60,20";
    
    translate_sequence_package(inputsequence2);

    unsigned char inputsequence3[] = "SetSection,3,80,20";
    
    translate_sequence_package(inputsequence3); */







    
    /////////////////////////////////////////////

    RingbufHandle_t rb =
        xRingbufferCreate(1024, RINGBUF_TYPE_BYTEBUF);
    assert(rb);

    init_leds();

    xTaskCreate(parse_data_task, "parser", 4096, rb, 5, NULL);

    bluetooth_init(rb);

    
}