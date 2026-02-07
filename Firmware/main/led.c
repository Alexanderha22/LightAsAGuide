#include "led.h"

uint32_t LED_GPIO[] = {LEDC0_OUTPUT_IO, LEDC1_OUTPUT_IO, LEDC2_OUTPUT_IO, LEDC3_OUTPUT_IO};
ledc_timer_t LED_TIMERS[] = {LEDC0_TIMER, LEDC1_TIMER, LEDC2_TIMER, LEDC3_TIMER};
ledc_channel_t LED_CHANNELS[] = {LEDC0_CHANNEL, LEDC1_CHANNEL, LEDC2_CHANNEL, LEDC3_CHANNEL};

Sequence StoredSequence;

//Global variables
Sequence StoredSequence;
int currentBlock;
float currentTimeStamp;
float nextTimeStamp;
int sequenceStarted;
int sequenceComplete;

void init_sequence(void)
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

void run_LED_sequence(void)
{

    //Check what time on timer 
    uint32_t resolution_hz;
    ESP_ERROR_CHECK(gptimer_get_resolution(gptimer, &resolution_hz));
    uint64_t count;
    ESP_ERROR_CHECK(gptimer_get_raw_count(gptimer, &count));
    float current_time = (double)count / resolution_hz;

    printf("Current Time: %f\n", current_time);

    if (currentTimeStamp <= current_time)
    {     

        printf("Current Time Stamp: %f\n", currentTimeStamp);
        printf("On Block Number: %d\n", currentBlock);
        printf("Going to next Time Stamp %f\n", nextTimeStamp);

        //Need to set LEDS
        //Loop through all light sections
        for (int lightNum = 0; lightNum < StoredSequence.N; lightNum++)
        {
            printf("Starting light frequencies\n");

            printf("lightNum: %i\n", lightNum);

            printf("total number of lights (N): %i\n", StoredSequence.N);

            printf("Setting to brightness: %f\n", StoredSequence.blocks[currentBlock].settings[lightNum].DutyCycle);
            printf("Setting to frequency: %f\n", StoredSequence.blocks[currentBlock].settings[lightNum].Frequency);

            float currDuty = StoredSequence.blocks[currentBlock].settings[lightNum].DutyCycle;
            float currFreq = StoredSequence.blocks[currentBlock].settings[lightNum].Frequency;

            //Check if frequency is 0, if yes then set to solid
            if (currFreq == 0)
            {   
                printf("Setting LED to solid\n");
                ledc_set_freq(LEDC_MODE, LED_TIMERS[lightNum], 100);
            }
            else
            {
                ledc_set_freq(LEDC_MODE, LED_TIMERS[lightNum], currFreq);
            }
            
            printf("Setting light %i frequency\n", lightNum);

            //Ramp LED if there is another value to go to (not on last block)
            if (currentBlock < StoredSequence.M - 1)
            {

                //Get next time stamp for future use
                nextTimeStamp = StoredSequence.blocks[currentBlock + 1].TimeStamp;

                float nextDuty = StoredSequence.blocks[currentBlock + 1].settings[lightNum].DutyCycle;

                //Need to just set duty cycle first (no change / fade)
                uint32_t duty = (pow(2, 13)) * (currDuty / 100);
                ledc_set_duty(LEDC_MODE, LED_CHANNELS[lightNum], duty);
                ledc_update_duty(LEDC_MODE, LED_CHANNELS[lightNum]);
                
                //Check if need to fade (change in brightness between sections)
                if (currDuty != nextDuty)
                {
                    //Start fade to ramp to next brightness
                    float nextTime = StoredSequence.blocks[currentBlock + 1].TimeStamp;
                    float duration = nextTime - current_time;

                    //Convert to millis
                    duration = duration * 1000;

                    //Find duty cycle from percent provided
                    //Equation example: Set duty to 50%: (2 ** 13) * 50% = 4096
                    uint32_t duty = (pow(2, 13)) * (nextDuty / 100);

                    //Starts fade
                    ledc_set_fade_with_time(LEDC_MODE, LED_CHANNELS[lightNum], duty, duration);
                    ledc_fade_start(LEDC_MODE, LED_CHANNELS[lightNum], LEDC_FADE_NO_WAIT);
                }

                printf("Setting light %i brightness\n", lightNum);
            }
            else
            {
                //On last block
                sequenceComplete = 1;

                //Set current duty cycle
                uint32_t duty = (pow(2, 13)) * (currDuty / 100);
                ledc_set_duty(LEDC_MODE, LED_CHANNELS[lightNum], duty);
                ledc_update_duty(LEDC_MODE, LED_CHANNELS[lightNum]);
            }
        }


        //Go to next block
        currentTimeStamp = nextTimeStamp;
        currentBlock++;
    }
}

void init_leds(void)
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

    //Setup for fade
    ledc_fade_func_install(0);

    //Turn off all LEDs to start
    for (int i = 0; i < 4; i++)
    {
        ledc_set_duty(LEDC_MODE, LED_CHANNELS[i], 0);
        ledc_update_duty(LEDC_MODE, LED_CHANNELS[i]);
    }
}

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
    else if (strcmp("SetSection", (char*)command) == 0)
    {
        //Convert command to c-style string
        char SequenceString[strlen((char*)sequence) + 1];
        strcpy(SequenceString, (char*)sequence);

        //Goes to command
        char* strPtr = strtok(SequenceString, ",");

        //Stores command
        StoredSequence.Command = malloc(strlen(strPtr) + 1);
        strcpy(StoredSequence.Command, strPtr);

        //Next goes to light number
        strPtr = strtok(NULL, ",");
        int lightNum = atoi(strPtr);

        printf("Setting section number %i\n", lightNum);

        //Next goes to brightness
        strPtr = strtok(NULL, ",");
        float givenDuty = atoi(strPtr);

        //Set duty
        
        uint32_t duty = (pow(2, 13)) * (givenDuty / 100);
        ledc_set_duty(LEDC_MODE, LED_CHANNELS[lightNum], duty);
        ledc_update_duty(LEDC_MODE, LED_CHANNELS[lightNum]);
        printf("Setting Brightness to %f\n", givenDuty);

        //Next goes to frequency
        strPtr = strtok(NULL, ",");
        float frequency = atoi(strPtr);

        //Set frequency
        if (frequency == 0)
        {   
            printf("Setting LED to solid (0 frequency)\n");
            ledc_set_freq(LEDC_MODE, LED_TIMERS[lightNum], 100);
        }
        else
        {
            printf("Setting frequency to %f\n", frequency);
            ledc_set_freq(LEDC_MODE, LED_TIMERS[lightNum], frequency);
        }
    }
    else if (strcmp("GetInfo", (char*)command) == 0)
    {
        //Need to send info back to APP about hard coded variables
        //Format:
        /*"SetInfo,m,n,
        L1x, L1y, L1s,
        L2x, L2y, L2s,
        …
        Lmx, Lmy, Lms,
        EID, Input/Output, Name, Description"*/
        //Where m = total number of lights
        //n = number of controllable sections
        //s = section light is part of
        //X & Y = position of light relative to (0,0) at center (VERIFY)
        

    }

}

void turn_off_leds(void)
{
    //Turn off all LEDs to start
    for (int i = 0; i < 4; i++)
    {
        ledc_set_duty(LEDC_MODE, LED_CHANNELS[i], 0);
        ledc_update_duty(LEDC_MODE, LED_CHANNELS[i]);
    }
}