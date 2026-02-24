#include "led.h"
#include "bluedroid_spp.h"

uint32_t LED_GPIO[] = {LEDC0_OUTPUT_IO, LEDC1_OUTPUT_IO, LEDC2_OUTPUT_IO, LEDC3_OUTPUT_IO};
ledc_timer_t LED_TIMERS[] = {LEDC0_TIMER, LEDC1_TIMER, LEDC2_TIMER, LEDC3_TIMER};
ledc_channel_t LED_CHANNELS[] = {LEDC0_CHANNEL, LEDC1_CHANNEL, LEDC2_CHANNEL, LEDC3_CHANNEL};

Sequence StoredSequence;

//Global variables
Sequence StoredSequence;
int currentBlock;
float currentTimeStamp;
float nextTimeStamp;
float sequenceStartTime;
int sequenceStarted;
int sequenceComplete;
FSMState GlobalState;

//LED setting variables
LEDSettings Section0Settings = {0, 0, 0};
LEDSettings Section1Settings = {0, 0, 0};
LEDSettings Section2Settings = {0, 0, 0};
LEDSettings Section3Settings = {0, 0, 0};

//LED setting variables
LEDSettings* LED_SETTINGS[4] = {&Section0Settings, &Section1Settings, &Section2Settings, &Section3Settings};




LightLocation LEDlocations[NUM_LIGHTS];

void init_sequence(void)
{

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
    float current_time = get_current_time();

    printf("Current Time: %f\n", current_time);

    if (currentTimeStamp <= current_time - sequenceStartTime) //See if enough time from start of sequence has passed
    {     

        //Go to next timestamp if not on last block
        if (currentBlock < StoredSequence.M - 1)
        {
            //Update next timestamp
            nextTimeStamp = StoredSequence.blocks[currentBlock + 1].TimeStamp;
        }


        printf("Current Time Stamp: %f\n", currentTimeStamp);
        printf("On Block Number: %d\n", currentBlock);
        printf("Going to next Time Stamp %f\n", nextTimeStamp);


        
        //Stop any lingering fade from previous block
        ledc_stop(LEDC_MODE, LED_CHANNELS[0], 0);
        ledc_stop(LEDC_MODE, LED_CHANNELS[1], 0);
        ledc_stop(LEDC_MODE, LED_CHANNELS[2], 0);
        ledc_stop(LEDC_MODE, LED_CHANNELS[3], 0);

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

            //Delay before doing duty
            //vTaskDelay(1);

            //Ramp LED if there is another value to go to (not on last block)
            if (currentBlock < StoredSequence.M - 1)
            {

                printf("Passed not on last block\n");

                float nextDuty = StoredSequence.blocks[currentBlock + 1].settings[lightNum].DutyCycle;

                //Need to just set duty cycle first (no change / fade)
                uint32_t duty = (pow(2, 13)) * (currDuty / 100);

                printf("Updating Duty\n");


                printf("Setting duty to %" PRIu32 "\n", duty);


                //DEBUG
                /* printf("Calling set_duty on ch: %d\n", LED_CHANNELS[lightNum]);
                esp_err_t err = ledc_set_duty(LEDC_MODE, LED_CHANNELS[lightNum], duty);
                printf("set_duty returned %d\n", err); */



                ledc_set_duty(LEDC_MODE, LED_CHANNELS[lightNum], duty);

                printf("finished set_duty\n");

                ledc_update_duty(LEDC_MODE, LED_CHANNELS[lightNum]);

                printf("Done updating duty\n");
                
                //Check if need to fade (change in brightness between sections)
                if (currDuty != nextDuty)
                {

                    printf("Starting fade calculation\n");

                    //Start fade to ramp to next brightness
                    //float nextTime = StoredSequence.blocks[currentBlock + 1].TimeStamp;
                    float duration = nextTimeStamp - (current_time - sequenceStartTime);

                    //Convert to millis
                    duration = duration * 1000;

                    printf("Fade Duration (ms): %f\n", duration);

                    //Find duty cycle from percent provided
                    //Equation example: Set duty to 50%: (2 ** 13) * 50% = 4096
                    uint32_t duty = (pow(2, 13)) * (nextDuty / 100);

                    printf("Setting fade\n");

                    //Starts fade
                    //ledc_set_fade_with_time(LEDC_MODE, LED_CHANNELS[lightNum], duty, duration);
                    //ledc_fade_start(LEDC_MODE, LED_CHANNELS[lightNum], LEDC_FADE_NO_WAIT);
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

                //Send SessionEnd to app
                //bt_write("SessionEnd", 10);

                //Go back to standby
                GlobalState = STANDBY;

                return;
            }
        }


        //Go to next block
        currentTimeStamp = nextTimeStamp;
        currentBlock++;
    }
}

void calculate_LED_settings(int lightNum, float duty, float frequency)
{

    //Calculate flashing period from given frequency
    float period;
    if (frequency != 0)
    {
        period = 1 / frequency;
    }
    else
    {
        period = -1;
    }

    printf("Saving LED settings\n");


    //Store the values
    LED_SETTINGS[lightNum]->duty = duty;
    LED_SETTINGS[lightNum]->period = period;
    LED_SETTINGS[lightNum]->start_time = get_current_time();

    printf("Done saving LED settings\n");

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

    //Establish LED locations and sections on board
    set_led_locations();
}

void init_state(void)
{
    GlobalState = STANDBY;

    // Create a timer instance
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));
    // Enable the timer
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    // Start the timer
    ESP_ERROR_CHECK(gptimer_start(gptimer));
}

void set_led_locations(void)
{
    //Set all LED locations and sections
    LEDlocations[0].x = 14;
    LEDlocations[0].y = 14;
    LEDlocations[0].section = 0;

    LEDlocations[1].x = 14;
    LEDlocations[1].y = -14;
    LEDlocations[1].section = 0;

    LEDlocations[2].x = -14;
    LEDlocations[2].y = -14;
    LEDlocations[2].section = 0;

    LEDlocations[3].x = -14;
    LEDlocations[3].y = 14;
    LEDlocations[3].section = 0;

    LEDlocations[4].x = 50;
    LEDlocations[4].y = 0;
    LEDlocations[4].section = 1;

    LEDlocations[5].x = 0;
    LEDlocations[5].y = 50;
    LEDlocations[5].section = 1;

    LEDlocations[6].x = -50;
    LEDlocations[6].y = 0;
    LEDlocations[6].section = 1;

    LEDlocations[7].x = 0;
    LEDlocations[7].y = -50;
    LEDlocations[7].section = 1;

    LEDlocations[8].x = -63;
    LEDlocations[8].y = 63;
    LEDlocations[8].section = 2;

    LEDlocations[9].x = -63;
    LEDlocations[9].y = 63;
    LEDlocations[9].section = 2;

    LEDlocations[10].x = -63;
    LEDlocations[10].y = -63;
    LEDlocations[10].section = 2;

    LEDlocations[11].x = 63;
    LEDlocations[11].y = 63;
    LEDlocations[11].section = 2;

    LEDlocations[12].x = 89;
    LEDlocations[12].y = 0;
    LEDlocations[12].section = 3;

    LEDlocations[13].x = 0;
    LEDlocations[13].y = 89;
    LEDlocations[13].section = 3;

    LEDlocations[14].x = -89;
    LEDlocations[14].y = 0;
    LEDlocations[14].section = 3;

    LEDlocations[15].x = 0;
    LEDlocations[15].y = -89;
    LEDlocations[15].section = 3;



}

void translate_command(unsigned char* sequence)
{
    //First check for commands with just one argument
    if (strcmp("GetInfo", (char*)sequence) == 0)
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

        printf("Get info command recieved\n");

        //Allocate enough space for each line
        int commandLen = (NUM_LIGHTS * 20) + (NUM_SECTIONS * 30) + 30;
        char returnCommand[commandLen];

        //Adds num_lights and num_sections
        sprintf(returnCommand, "SetInfo,%s,%s,\n", STR(NUM_LIGHTS), STR(NUM_SECTIONS));

        //Loop for all the lights to load their location and section
        for (int i = 0; i < NUM_LIGHTS; i++)
        {
            char buffer[20] = "";
            sprintf(buffer, "L%i%i,L%i%i,L%i%i,\n", i, LEDlocations[i].x, i, LEDlocations[i].y, i, LEDlocations[i].section);
            strcat(returnCommand, buffer);
        }

        //See if need to add anything about external modules
        if (NUM_MODULES > 0)
        {
            //IMPLEMENT CODE FOR MODULE INFO
        }

        //Send message via bluetooth here?
        bt_write(returnCommand, commandLen);
        

        printf("Resulting GetInfo command: %s", returnCommand);
    }

    else if (strcmp("StopAll", (char*)sequence) == 0)
    {
        //Turn off LEDS and reset them, stop sequence, reset state
        turn_off_leds();

        GlobalState = STANDBY;

    }

    else
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


            //Begin parsing for sequencing information

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

            //Save what time this command was recieved (start time)
            sequenceStartTime = get_current_time();

            //Have LEDS reset
            turn_off_leds();

            //Reset sequence data
            init_sequence();

            //Set global state to begin sequence task
            GlobalState = SEQUENCE;



        }
        else if (strcmp("SetSection", (char*)command) == 0)
        {
            //Check if currently in a sequence, if so, then send error 
            if (GlobalState == SEQUENCE)
            {
                bt_write("SendError, Stop current sequence before setting section.", 56);
            }
            else //Set the section
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
                /*ledc_set_duty(LEDC_MODE, LED_CHANNELS[lightNum], duty);
                ledc_update_duty(LEDC_MODE, LED_CHANNELS[lightNum]);*/
                printf("Setting Brightness to %f\n", givenDuty); 

                //Next goes to frequency
                strPtr = strtok(NULL, ",");
                float frequency = atoi(strPtr);

                //Set if solid or off (doesnt need to be handled by task for flashing)
                if (frequency == 0) //Solid
                {   
                    printf("Setting LED to solid (0 frequency)\n");
                    ledc_set_freq(LEDC_MODE, LED_TIMERS[lightNum], 100);
                    ledc_set_duty(LEDC_MODE, LED_CHANNELS[lightNum], duty);
                    ledc_update_duty(LEDC_MODE, LED_CHANNELS[lightNum]);
                }
                else if (givenDuty == 0) //Off
                {
                    ledc_set_duty(LEDC_MODE, LED_CHANNELS[lightNum], duty);
                    ledc_update_duty(LEDC_MODE, LED_CHANNELS[lightNum]);
                }

                //Updated LED settings
                calculate_LED_settings(lightNum, duty, frequency);

            }
            
        }
    }


    


}

void turn_off_leds(void)
{
    //Turn off all LEDs to start and reset frequencies to default
    for (int i = 0; i < 4; i++)
    {
        ledc_set_duty(LEDC_MODE, LED_CHANNELS[i], 0);
        ledc_update_duty(LEDC_MODE, LED_CHANNELS[i]);
        ledc_set_freq(LEDC_MODE, LED_TIMERS[i], 100);

    }

}

//Get current time on timer
float get_current_time()
{
    uint32_t resolution_hz;
    ESP_ERROR_CHECK(gptimer_get_resolution(gptimer, &resolution_hz));
    uint64_t count;
    ESP_ERROR_CHECK(gptimer_get_raw_count(gptimer, &count));
    float current_time = (double)count / resolution_hz;

    return current_time;
}
