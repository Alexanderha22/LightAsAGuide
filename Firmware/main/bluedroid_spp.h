#ifndef BLUEDROID_SPP_H
#define BLUEDROID_SPP_H

#include "freertos/ringbuf.h"

void bluetooth_init(RingbufHandle_t rb);
bool bt_write(char *str, int len);

#endif