/* LEDC (LED Controller) basic example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
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
#define LEDC0_DUTY               (4096) // Set duty to 50%. (2 ** 13) * 50% = 4096
#define LEDC0_FREQUENCY          (10) // Frequency in Hertz

//LED1 on GPIO17
#define LEDC1_TIMER              LEDC_TIMER_1
#define LEDC1_OUTPUT_IO          (17) // Define the output GPIO
#define LEDC1_CHANNEL            LEDC_CHANNEL_1
#define LEDC1_DUTY               (4096) // Set duty to 50%. (2 ** 13) * 50% = 4096
#define LEDC1_FREQUENCY          (40) // Frequency in Hertz

//LED2 on GPIO18
#define LEDC2_TIMER              LEDC_TIMER_2
#define LEDC2_OUTPUT_IO          (18) // Define the output GPIO
#define LEDC2_CHANNEL            LEDC_CHANNEL_2
#define LEDC2_DUTY               (819) // Set duty to 10%. (2 ** 13) * 50% = 4096
#define LEDC2_FREQUENCY          (10) // Frequency in Hertz. Set frequency at 4 kHz

//LED3 on GPIO19
#define LEDC3_TIMER              LEDC_TIMER_3
#define LEDC3_OUTPUT_IO          (19) // Define the output GPIO
#define LEDC3_CHANNEL            LEDC_CHANNEL_3
#define LEDC3_DUTY               (819) // Set duty to 50%. (2 ** 13) * 50% = 4096
#define LEDC3_FREQUENCY          (40) // Frequency in Hertz. Set frequency at 4 kHz


//General
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits

#define MAX_SEQUENCE_LENGTH     1000

const int LED_TIMERS[] = {LEDC0_TIMER, LEDC1_TIMER, LEDC2_TIMER, LEDC3_TIMER};
const int LED_CHANNELS[] = {LEDC0_CHANNEL, LEDC1_CHANNEL, LEDC2_CHANNEL, LEDC3_CHANNEL};


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

//Structs
typedef struct
{
    char* Command;
    int M;
    int N;
    Block* blocks; //Will be M number of entries for each block
} Sequence ;


//Global variables
Sequence StoredSequence;
int currentBlock;
float currentTimeStamp;
float nextTimeStamp;
int sequenceStarted;
int sequenceComplete;

//Timer Setup
gptimer_handle_t gptimer = NULL;
gptimer_config_t timer_config = {
    .clk_src = GPTIMER_CLK_SRC_DEFAULT, // Select the default clock source
    .direction = GPTIMER_COUNT_UP,      // Counting direction is up
    .resolution_hz = 1 * 1000 * 1000,   // Resolution is 1 MHz, i.e., 1 tick equals 1 microsecond
};


static TaskHandle_t s_worker_handle = NULL;
static QueueHandle_t s_data_queue = NULL;
static void worker_task(void *arg);

static void run_LED_sequence();


static void worker_task(void *arg)
{
    const TickType_t delay_ticks = pdMS_TO_TICKS(10);

    while (1)
    {
        if (sequenceStarted == 0)
        {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        while (sequenceStarted == 1)
        {
            run_LED_sequence();
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}




static void init_sequence(void)
{
    // Create a timer instance
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));
    // Enable the timer
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    // Start the timer
    ESP_ERROR_CHECK(gptimer_start(gptimer));

    //Start sequence
    currentBlock = 0;

    currentTimeStamp = 0;
    nextTimeStamp = StoredSequence.blocks[currentBlock + 1].TimeStamp;

    sequenceStarted = 1;
    sequenceComplete = 0;


}


//Use stored sequence and timer to enable/disable LEDs
void run_LED_sequence(void)
{
    //Check what time on timer 
    uint32_t resolution_hz;
    ESP_ERROR_CHECK(gptimer_get_resolution(gptimer, &resolution_hz));
    uint64_t count;
    ESP_ERROR_CHECK(gptimer_get_raw_count(gptimer, &count));
    float current_time = (double)count / resolution_hz;

    if (currentTimeStamp == current_time)
    {     

        printf("Current Time Stamp: %f\n", current_time);
        printf("Block Number: %d\n", currentBlock);

        currentTimeStamp = nextTimeStamp;

        //Need to set LEDS
        //Loop through all light sections
        for (int lightNum = 0; lightNum < StoredSequence.N; lightNum++)
        {
            ledc_set_freq(LEDC_MODE, LED_TIMERS[lightNum], StoredSequence.blocks[currentBlock].settings[lightNum].Frequency);
            printf("Setting light %i frequency\n", lightNum);

            //Ramp LED if there is another value to go to (not on last block)
            if (currentBlock < StoredSequence.M - 1)
            {
                
                //Get time until next block
                nextTimeStamp = StoredSequence.blocks[currentBlock + 1].TimeStamp;
                float duration = nextTimeStamp - current_time;

                //Convert to millis
                duration = duration / 1000;

                //Find duty cycle from percent provided
                // Set duty to 50%. (2 ** 13) * 50% = 4096
                uint32_t duty = (pow(2, 13)) * (StoredSequence.blocks[currentBlock].settings[lightNum].DutyCycle / 100);

                ledc_set_fade_with_time(LEDC_MODE, LED_CHANNELS[lightNum], duty, duration);

                printf("Setting light %i brightness\n", lightNum);
            }
            else
            {
                sequenceStarted = 1;
            }
        }


        //Go to next block
        currentBlock++;
    }
}



/* Warning:
 * For ESP32, ESP32S2, ESP32S3, ESP32C3, ESP32C2, ESP32C6, ESP32H2 (rev < 1.2), ESP32P4 targets,
 * when LEDC_DUTY_RES selects the maximum duty resolution (i.e. value equal to SOC_LEDC_TIMER_BIT_WIDTH),
 * 100% duty cycle is not reachable (duty cannot be set to (2 ** SOC_LEDC_TIMER_BIT_WIDTH)).
 */

static void example_ledc_init(void)
{
    // LED0
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc0_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC0_TIMER,
        .freq_hz          = LEDC0_FREQUENCY,  // Set output frequency at 4 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc0_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc0_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC0_CHANNEL,
        .timer_sel      = LEDC0_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC0_OUTPUT_IO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc0_channel));

    // LED1
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc1_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC1_TIMER,
        .freq_hz          = LEDC1_FREQUENCY,  // Set output frequency at 4 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc1_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc1_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC1_CHANNEL,
        .timer_sel      = LEDC1_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC1_OUTPUT_IO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc1_channel));

    // LED2
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc2_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC2_TIMER,
        .freq_hz          = LEDC2_FREQUENCY,  // Set output frequency at 4 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc2_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc2_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC2_CHANNEL,
        .timer_sel      = LEDC2_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC2_OUTPUT_IO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc2_channel));

    // LED3
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc3_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC3_TIMER,
        .freq_hz          = LEDC3_FREQUENCY,  // Set output frequency at 4 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc3_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc3_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC3_CHANNEL,
        .timer_sel      = LEDC3_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC3_OUTPUT_IO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc3_channel));
}

//Divide up sequencing information from Bluetooth to convert to frequency and duty cycle
void translate_sequence_package(unsigned char* sequence)
{

    //Check for sequence size
    size_t sequence_size = strlen((char*)sequence);

    if (sequence_size < 1)
    {
        return;
    }

    size_t command_size = 0;

    //Check for type of command
    //Need to look at first section before first comma
    for (int i = 0; i < sequence_size; i++)
    {
        if (sequence[i] == 44) //Comma ASCII code
        {
            command_size = i;
            break;
        }
    }

    //Copy over command
    unsigned char command[command_size + 1];

    for (int i = 0; i < command_size; i++)
    {
        command[i] = sequence[i];
    }
    
    command[command_size] = '\0';

    //Check for command
    if (strcmp("SendSession", (char*)command) == 0)
    {
        //Parsing for sequencing information

        //Convert command to c-style string
        char SequenceString[strlen((char*)sequence) + 1];
        strcpy(SequenceString, (char*)sequence);

        //Get first line of sequence before line break character
        char* firstLine = strtok(SequenceString, "\n");

        //Parse 3 times to split command, M, and N
        char* strPtr = strtok(firstLine, ",");
        
        //First is command, store
        StoredSequence.Command = malloc(strlen(strPtr) + 1);
        strcpy(StoredSequence.Command, strPtr);

        //Move to next one
        strPtr = strtok(NULL, ",");

        //Store second value (M)
        StoredSequence.M = atoi(strPtr);

        //Move to next one
        strPtr = strtok(NULL, ",");

        //Store thrid value (N)
        StoredSequence.N = atoi(strPtr);

        //Get sequence again
        strcpy(SequenceString, (char*)sequence);

        strPtr = strtok(SequenceString, "\n");

        //Move to next one since we do not need first line
        strPtr = strtok(NULL, "\n");

        //Keep track of what block we are on
        int blockNum = 0;

        //Array to hold all the blocks as strings
        char blockStrings[StoredSequence.M][((StoredSequence.N * 2) + 1) * 8];

        //Collects all the blocks as strings
        while(strPtr != NULL)
        {

            strcpy(blockStrings[blockNum], strPtr);

            //Move to next block
            strPtr = strtok(NULL, "\n");

            //Increment
            blockNum++;
        
        }

        //Need to allocate space for block array in stored sequence
        StoredSequence.blocks = malloc(StoredSequence.M * sizeof(Block));

        //Need to loop through all blocks
        for (int i = 0; i < StoredSequence.M; i++)
        {
            char blockString[strlen(blockStrings[i]) + 1];
            strcpy(blockString, blockStrings[i]);

            //Need to parse the blockstring by , to get timestamp and settings
            char* blockPtr = strtok(blockString, ",");

            //First is the time stamp, need to store
            StoredSequence.blocks[i].TimeStamp = atof(blockPtr);

            //Need to loop through all light settings
            //Need to allocate space for settings
            StoredSequence.blocks[i].settings = malloc(StoredSequence.N * sizeof(LightSetting));
            for (int j = 0; j < StoredSequence.N; j++)
            {
                //Next parse is setting for brightness
                blockPtr = strtok(NULL, ",");
                StoredSequence.blocks[i].settings[j].DutyCycle = atof(blockPtr);

                //Next parse is setting for frequency
                blockPtr = strtok(NULL, ",");
                StoredSequence.blocks[i].settings[j].Frequency = atof(blockPtr);
            }
        }

    }

}



/* example
void app_main(void)
{
    // Set the LEDC peripheral configuration
    example_ledc_init();

    // Set duty cycle
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC0_CHANNEL, LEDC0_DUTY));
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC1_CHANNEL, LEDC1_DUTY));
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC2_CHANNEL, LEDC2_DUTY));
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC3_CHANNEL, LEDC3_DUTY));

    // Update duty to apply the new value
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC1_CHANNEL));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC1_CHANNEL));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC1_CHANNEL));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC1_CHANNEL));

    

}
    */
