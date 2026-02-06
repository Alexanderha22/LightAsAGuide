/* LEDC (LED Controller) basic example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#ifndef LED_H
#define LED_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "esp_mac.h"
#include "driver/ledc.h"
#include "esp_err.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gptimer.h"
#include "esp_log.h"


//LED0 on GPIO16
#define LEDC0_TIMER              LEDC_TIMER_0
#define LEDC0_OUTPUT_IO          (16) // Define the output GPIO
#define LEDC0_CHANNEL            LEDC_CHANNEL_0
#define LEDC0_DUTY               (0) // Set duty to 0%. (2 ** 13) * 50% = 4096
#define LEDC0_FREQUENCY          (10) // Frequency in Hertz

//LED1 on GPIO17
#define LEDC1_TIMER              LEDC_TIMER_1
#define LEDC1_OUTPUT_IO          (17) // Define the output GPIO
#define LEDC1_CHANNEL            LEDC_CHANNEL_1
#define LEDC1_DUTY               (0) // Set duty to 0%. (2 ** 13) * 50% = 4096
#define LEDC1_FREQUENCY          (10) // Frequency in Hertz

//LED2 on GPIO18
#define LEDC2_TIMER              LEDC_TIMER_2
#define LEDC2_OUTPUT_IO          (18) // Define the output GPIO
#define LEDC2_CHANNEL            LEDC_CHANNEL_2
#define LEDC2_DUTY               (0) // Set duty to 0%. (2 ** 13) * 50% = 4096
#define LEDC2_FREQUENCY          (10) // Frequency in Hertz. Set frequency at 4 kHz

//LED3 on GPIO19
#define LEDC3_TIMER              LEDC_TIMER_3
#define LEDC3_OUTPUT_IO          (19) // Define the output GPIO
#define LEDC3_CHANNEL            LEDC_CHANNEL_3
#define LEDC3_DUTY               (0) // Set duty to 0%. (2 ** 13) * 50% = 4096
#define LEDC3_FREQUENCY          (10) // Frequency in Hertz. Set frequency at 4 kHz


//General
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits

#define MAX_SEQUENCE_LENGTH     1000
#define MAX_FREQUENCY           100

extern uint32_t LED_GPIO[];
extern ledc_timer_t LED_TIMERS[];
extern ledc_channel_t LED_CHANNELS[];

//Structs to define sequence settings
typedef struct
{
    float DutyCycle;
    float Frequency;
} LightSetting;

typedef struct
{
    float TimeStamp;
    LightSetting* settings; //Will be N number of entries for each light section
} Block ;

typedef struct
{
    char* Command;
    int M;
    int N;
    Block* blocks; //Will be M number of entries for each block
} Sequence ;




//Timer Setup
static gptimer_handle_t gptimer = NULL;
static gptimer_config_t timer_config = {
    .clk_src = GPTIMER_CLK_SRC_DEFAULT, // Select the default clock source
    .direction = GPTIMER_COUNT_UP,      // Counting direction is up
    .resolution_hz = 1 * 1000 * 1000,   // Resolution is 1 MHz, i.e., 1 tick equals 1 microsecond
};

void run_LED_sequence(void);


//Initializes sequence variables
void init_sequence(void);


//Use stored sequence and timer to enable/disable LEDs
void run_LED_sequence(void);



/* Warning:
 * For ESP32, ESP32S2, ESP32S3, ESP32C3, ESP32C2, ESP32C6, ESP32H2 (rev < 1.2), ESP32P4 targets,
 * when LEDC_DUTY_RES selects the maximum duty resolution (i.e. value equal to SOC_LEDC_TIMER_BIT_WIDTH),
 * 100% duty cycle is not reachable (duty cannot be set to (2 ** SOC_LEDC_TIMER_BIT_WIDTH)).
 */

void init_leds(void);

//Divide up sequencing information from Bluetooth to convert to frequency and duty cycle
void translate_sequence_package(unsigned char* sequence);

#endif


//SEQUENCE
//unsigned char inputsequence[] = "SendSession,3,4,\n0.0,50.0,2.0,50.0,4.0,50.0,8.0,50.0,16.0,\n5.0,50.0,0.0,50.0,4.0,50.0,8.0,50.0,16.0,\n10.0,50.0,2.0,50.0,4.0,50.0,8.0,50.0,16.0";