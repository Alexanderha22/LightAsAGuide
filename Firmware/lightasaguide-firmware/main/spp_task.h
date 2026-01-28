#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/stream_buffer.h"
#include "freertos/queue.h"

void parseData_Task(void *args)
{
    for(;;)
    {
        uint8_t ucRxData[ 20 ];

        size_t xReceivedBytes;

        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        if(pdTRUE == xStreamBufferReceiveFromISR()) {

            

        } else {

        }
    }
    vTaskDelete( NULL );
}