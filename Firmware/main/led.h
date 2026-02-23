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
#define LEDC0_FREQUENCY          (100) // Frequency in Hertz

//LED1 on GPIO17
#define LEDC1_TIMER              LEDC_TIMER_1
#define LEDC1_OUTPUT_IO          (17) // Define the output GPIO
#define LEDC1_CHANNEL            LEDC_CHANNEL_1
#define LEDC1_DUTY               (0) // Set duty to 0%. (2 ** 13) * 50% = 4096
#define LEDC1_FREQUENCY          (100) // Frequency in Hertz

//LED2 on GPIO18
#define LEDC2_TIMER              LEDC_TIMER_2
#define LEDC2_OUTPUT_IO          (18) // Define the output GPIO
#define LEDC2_CHANNEL            LEDC_CHANNEL_2
#define LEDC2_DUTY               (0) // Set duty to 0%. (2 ** 13) * 50% = 4096
#define LEDC2_FREQUENCY          (100) // Frequency in Hertz. Set frequency at 4 kHz

//LED3 on GPIO19
#define LEDC3_TIMER              LEDC_TIMER_3
#define LEDC3_OUTPUT_IO          (19) // Define the output GPIO
#define LEDC3_CHANNEL            LEDC_CHANNEL_3
#define LEDC3_DUTY               (0) // Set duty to 0%. (2 ** 13) * 50% = 4096
#define LEDC3_FREQUENCY          (100) // Frequency in Hertz. Set frequency at 4 kHz


//General
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits

#define MAX_SEQUENCE_LENGTH     1000
#define MAX_FREQUENCY           100

//HARDWARE SPECIFIC PARAMETERS:
#define NUM_LIGHTS              16 //Number of lights for this specific hardware
#define NUM_SECTIONS            4  //Number of controllable sections
#define NUM_MODULES             0  //Num of additional input/output modules

//String helper to convert num_lights into string later on
//From: https://stackoverflow.com/questions/5459868/concatenate-int-to-string-using-c-preprocessor
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

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



//Struct to hold information for each light and its section/locations
typedef struct
{
    int x;
    int y;
    int section;
} LightLocation ;

//Enum to contain which state the program is in
typedef enum
{
    SEQUENCE,
    STANDBY
} FSMState ;

extern FSMState GlobalState;

//LED settings for live control to be stored here
//Used by run_leds_task
typedef struct
{
    uint32_t duty;
    float period;
    float start_time;
} LEDSettings;

extern LEDSettings Section0Settings;
extern LEDSettings Section1Settings;
extern LEDSettings Section2Settings;
extern LEDSettings Section3Settings;

extern LEDSettings* LED_SETTINGS[4];




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

//Calculate the correct flashing period and brightness when recieving info for setsection
void calculate_LED_settings(int lightNum, float givenDuty, float frequency);




/* Warning:
 * For ESP32, ESP32S2, ESP32S3, ESP32C3, ESP32C2, ESP32C6, ESP32H2 (rev < 1.2), ESP32P4 targets,
 * when LEDC_DUTY_RES selects the maximum duty resolution (i.e. value equal to SOC_LEDC_TIMER_BIT_WIDTH),
 * 100% duty cycle is not reachable (duty cannot be set to (2 ** SOC_LEDC_TIMER_BIT_WIDTH)).
 */

void init_leds(void);

void init_state(void);

void set_led_locations(void);

//Divide up sequencing information from Bluetooth to convert to frequency and duty cycle
void translate_command(unsigned char* sequence);

void turn_off_leds(void);

float get_current_time();

#endif


//SEQUENCE
//unsigned char inputsequence[] = "SendSession,3,4,\n0.0,50.0,2.0,50.0,4.0,50.0,8.0,50.0,16.0,\n5.0,50.0,0.0,50.0,4.0,50.0,8.0,50.0,16.0,\n10.0,50.0,2.0,50.0,4.0,50.0,8.0,50.0,16.0";