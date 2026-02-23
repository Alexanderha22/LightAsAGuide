#include "run_leds_task.h"

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/stream_buffer.h"
#include "freertos/ringbuf.h"
#include "freertos/queue.h"
#include "esp_spp_api.h"

#include "led.h"


void run_leds_task(void *arg)
{
    while(1)
    {
        if (GlobalState == SEQUENCE)
        {
            //Run the sequece
            run_LED_sequence();
        }
        else if (GlobalState == STANDBY)
        {
            //Loop for the LEDs to have independent frequency and PWM
            for (int i = 0; i < NUM_SECTIONS; i++)
            {
                //Check if on and flashing
                if (LED_SETTINGS[i]->duty > 0 && LED_SETTINGS[i]->period != -1)
                {
                    //Check if flashing (first half of period) and need to be set to appropriate pwm for brightness
                    if (fmod(get_current_time() - LED_SETTINGS[i]->start_time, LED_SETTINGS[i]->period) < LED_SETTINGS[i]->period / 2)
                    {
                        //Set to pwm
                        ledc_set_duty(LEDC_MODE, LED_CHANNELS[i], LED_SETTINGS[i]->duty);
                        ledc_update_duty(LEDC_MODE, LED_CHANNELS[i]);
                    }
                    else
                    {
                        //Set to off
                        ledc_set_duty(LEDC_MODE, LED_CHANNELS[i], 0);
                        ledc_update_duty(LEDC_MODE, LED_CHANNELS[i]);
                    }
                }
            }

        }
        vTaskDelay(5);
    }
}
