#include "led.h"
#include "bluedroid_spp.h"

//Global arrays to access all LED sections
uint32_t LED_GPIO[] = {LEDC0_OUTPUT_IO, LEDC1_OUTPUT_IO, LEDC2_OUTPUT_IO, LEDC3_OUTPUT_IO};
ledc_timer_t LED_TIMERS[] = {LEDC0_TIMER, LEDC1_TIMER, LEDC2_TIMER, LEDC3_TIMER};
ledc_channel_t LED_CHANNELS[] = {LEDC0_CHANNEL, LEDC1_CHANNEL, LEDC2_CHANNEL, LEDC3_CHANNEL};


//Global variables
Sequence StoredSequence;
int currentBlock;
float currentTimeStamp;
float nextTimeStamp;
float sequenceStartTime;
int sequenceStarted;
FSMState GlobalState;
int activeRelayGPIO = 4;
RelayState relayState = OFF;

//LED setting variables
LEDSettings Section0Settings = {0, 0, 0};
LEDSettings Section1Settings = {0, 0, 0};
LEDSettings Section2Settings = {0, 0, 0};
LEDSettings Section3Settings = {0, 0, 0};

//LED setting variables
LEDSettings* LED_SETTINGS[4] = {&Section0Settings, &Section1Settings, &Section2Settings, &Section3Settings};

LightLocation LEDlocations[NUM_LIGHTS];


//Timer Setup
gptimer_handle_t gptimer = NULL;
gptimer_config_t timer_config = {
    .clk_src = GPTIMER_CLK_SRC_DEFAULT, // Select the default clock source
    .direction = GPTIMER_COUNT_UP,      // Counting direction is up
    .resolution_hz = 1 * 1000 * 1000,   // Resolution is 1 MHz, i.e., 1 tick equals 1 microsecond
};

//ESPTimer variable
esp_timer_handle_t oneshot_timer;


void init_sequence(void)
{

    //Have LEDS reset
    turn_off_leds();


    //Save what time this command was recieved (start time)
    sequenceStartTime = get_current_time();

    //Set global state to begin sequence task
    GlobalState = SEQUENCE;

    //Start sequence
    currentBlock = 0;

    currentTimeStamp = 0;
    nextTimeStamp = StoredSequence.blocks[currentBlock + 1].TimeStamp;

    sequenceStarted = 1;


}


void reset_sequence(void)
{

    //Free space from malloc


    for (int i = 0; i < StoredSequence.M; i++)
    {
        free(StoredSequence.blocks[i].settings);
    }

    free(StoredSequence.blocks);
    
    
}

void run_LED_sequence(void)
{

    //Check what the current time is on timer 
    float current_time = get_current_time();

    printf("Current Time: %f\n", current_time);

    printf("Difference between start and current: %f\n", current_time - sequenceStartTime);

    if (currentTimeStamp <= current_time - sequenceStartTime) //See if enough time from start of sequence has passed
    {     

        printf("Current Time Stamp: %f\n", currentTimeStamp);
        printf("On Block Number: %d\n", currentBlock);

        //Go to and store next timestamp if not on last block
        if (currentBlock < StoredSequence.M - 1)
        {
            //Update next timestamp
            nextTimeStamp = StoredSequence.blocks[currentBlock + 1].TimeStamp;
            printf("Going to next Time Stamp %f\n", nextTimeStamp);
        }
        else //On last block
        {
            //Turn off all LEDS and finish sequnece
            turn_off_leds();
            reset_sequence();
            init_sequence();
            GlobalState = STANDBY;

            //Send SessionEnd to app
            bt_write("SessionEnd", 10);
            printf("Session ended\n");

            return;

        }

        //Send timestamp to app
        char sendTimeStamp[32];

        sprintf(sendTimeStamp, "Timestamp,%0.2d", currentTimeStamp);

        bt_write(sendTimeStamp, strlen(sendTimeStamp));


        //Need to set LEDS
        //Loop through all light sections
        for (int lightNum = 0; lightNum < StoredSequence.N; lightNum++)
        {
             /*  printf("Starting light frequencies\n");

                printf("lightNum: %i\n", lightNum);

                printf("total number of lights (N): %i\n", StoredSequence.N);

             */

            float currDuty = StoredSequence.blocks[currentBlock].settings[lightNum].DutyCycle;
            float currFreq = StoredSequence.blocks[currentBlock].settings[lightNum].Frequency;

            printf("Setting to brightness: %f\n", currDuty);
            printf("Setting to frequency: %f\n", currFreq);

            

            calculate_LED_settings(lightNum, currDuty, currFreq, current_time);


            /* //Ramp LED if there is another value to go to (not on last block)
            if (currentBlock < StoredSequence.M - 1)
            {

                printf("Passed not on last block\n");

                float nextDuty = StoredSequence.blocks[currentBlock + 1].settings[lightNum].DutyCycle;
                
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
                    //uint32_t duty = (pow(2, 13)) * (nextDuty / 100);

                    printf("Setting fade\n");

                    //Starts fade
                    //ledc_set_fade_with_time(LEDC_MODE, LED_CHANNELS[lightNum], duty, duration);
                    //ledc_fade_start(LEDC_MODE, LED_CHANNELS[lightNum], LEDC_FADE_NO_WAIT);
                }

                printf("Setting light %i brightness\n", lightNum);
            } */

        }

        //Go to next block
        currentTimeStamp = nextTimeStamp;
        currentBlock++;
    }
}

void calculate_LED_settings(int lightNum, float duty, float frequency, float startTime)
{

    //Calculate flashing period from given frequency
    float period;
    if (frequency != 0)
    {
        period = 1 / frequency;
    }
    else
    {
        period = -1; //Solid
    }

    printf("Saving LED settings\n");


    //Store the values
    LED_SETTINGS[lightNum]->duty = (pow(2, 13)) * (duty / 100);
    LED_SETTINGS[lightNum]->period = period;
    LED_SETTINGS[lightNum]->start_time = startTime;

    printf("Done saving LED settings\n");

    //Turn on solid if not flashing so only has to do so once
    if ((period == -1) && (duty > 0))
    {
        ledc_set_duty(LEDC_MODE, LED_CHANNELS[lightNum], LED_SETTINGS[lightNum]->duty);
        ledc_update_duty(LEDC_MODE, LED_CHANNELS[lightNum]);
    }


    //Check if all settings are 0 brightness, if yes turn off relay
    //Also check if we need to turn on the relay
    bool allzero = true;
    for (int i = 0; i < NUM_SECTIONS; i++)
    {
        if (LED_SETTINGS[i]->duty != 0)
        {
            allzero = false;
        }
    }

    if (allzero && relayState == ON)
    {
        //turn_off_relay();
    }
    else if (!allzero && relayState == OFF)
    {
        turn_on_relay();
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

    //Establish LED locations and sections on board
    set_led_locations();
}

void init_state(void)
{
    GlobalState = STANDBY;
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

    LEDlocations[9].x = 63;
    LEDlocations[9].y = -63;
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

    //Check if input even exists
    if (sequence == NULL)
    {
        bt_write("Error: No Command", 17);
        printf("ERROR: Received NULL from command\n");
        return;
    }



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
            sprintf(buffer, "L%i.%i.%i.%i,\n", i, LEDlocations[i].x, LEDlocations[i].y, LEDlocations[i].section);
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

        turn_off_relay();

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
            
            //First is command
            

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
            char** blockStrings = malloc(StoredSequence.M * sizeof(char*));

            //Collects all the blocks as strings until all blocks 
            while(strPtr != NULL && blockNum < StoredSequence.M)
            {

                blockStrings[blockNum] = strdup(strPtr);

                //Move to next block
                strPtr = strtok(NULL, "\n");

                //Increment
                blockNum++;
            
            }

            //Check if provided lines matches up with given M
            if (blockNum != StoredSequence.M)
            {
                printf("Mismatch in sequence lines and block number\n");
                return;
            }

            //Need to allocate space for block array in stored sequence
            StoredSequence.blocks = malloc(StoredSequence.M * sizeof(Block));

            //Need to loop through all blocks to parse each one
            for (int i = 0; i < StoredSequence.M; i++)
            {
                //char blockString[strlen(blockStrings[i]) + 1];
                char* blockString = strdup(blockStrings[i]);




                //Need to parse the blockstring by , to get timestamp and settings
                char* blockPtr = strtok(blockString, ",");

                if (!blockPtr)
                {
                    printf("NULL for blockptr1\n");
                }

                //First is the time stamp, need to store
                StoredSequence.blocks[i].TimeStamp = atof(blockPtr);

                //Need to loop through all light settings
                //Need to allocate space for settings
                StoredSequence.blocks[i].settings = malloc(StoredSequence.N * sizeof(LightSetting));
                for (int j = 0; j < StoredSequence.N; j++)
                {
                    //Next parse is setting for brightness
                    blockPtr = strtok(NULL, ",");

                    if (!blockPtr)
                    {
                        printf("NULL for blockptr2, i = %i\n", i);
                    }

                    StoredSequence.blocks[i].settings[j].DutyCycle = atof(blockPtr);

                    //Next parse is setting for frequency
                    blockPtr = strtok(NULL, ",");

                    if (!blockPtr)
                    {
                        printf("NULL for blockptr3, i = %i\n", i);
                    }

                    StoredSequence.blocks[i].settings[j].Frequency = atof(blockPtr);
                }

                //Free from the malloc within calling strdup
                free(blockString);

            }

            //Free all strings allocated with strdup
            for (int i = 0; i < StoredSequence.M; i++)
            {
                free(blockStrings[i]);
            }
            free(blockStrings);



            //Initializes sequence data
            init_sequence();





        }
        else if (strcmp("SetSection", (char*)command) == 0)
        {
            //Check if currently in a sequence, if so, then send error 
            if (GlobalState == SEQUENCE)
            {
                bt_write("SendError, Stop current sequence before setting section.", 56);
                printf("Tried to set section when sequence was running\n");
            }
            else //Set the section
            {
                //Convert command to c-style string
                char SequenceString[strlen((char*)sequence) + 1];
                strcpy(SequenceString, (char*)sequence);

                printf("SequenceString: %s\n", SequenceString);
                printf("original sequence: %s\n", (char*)sequence);

                //Goes to command
                char* strPtr = strtok(SequenceString, ",");

                printf("StrPtr: %s\n", strPtr);

                if (strPtr == NULL)
                {
                    printf("ERROR NULL strtok\n");
                    return;
                }

                //Next goes to brightness
                strPtr = strtok(NULL, ",");

                printf("StrPtr: %s\n", strPtr);

                if (strPtr == NULL)
                {
                    printf("ERROR NULL strtok\n");
                }

                float givenDuty = atof(strPtr);

                //Set duty
                
                /*ledc_set_duty(LEDC_MODE, LED_CHANNELS[lightNum], duty);
                ledc_update_duty(LEDC_MODE, LED_CHANNELS[lightNum]);*/
                printf("Setting Brightness to %f\n", givenDuty); 

                //Next goes to frequency
                strPtr = strtok(NULL, ",");

                printf("StrPtr: %s\n", strPtr);

                if (strPtr == NULL)
                {
                    printf("ERROR NULL strtok\n");
                    printf("Original sequence: %s\n", (char*)sequence);
                }

                float frequency = atof(strPtr);

                
                printf("Setting Frequency to %f\n", frequency);

                //Then goes to all selected sections
                while ((strPtr = strtok(NULL, ",")) != NULL)
                {

                    printf("StrPtr: %s\n", strPtr);

                    if (strPtr == NULL)
                    {
                        printf("ERROR NULL strtok\n");
                    }

                    int lightNum = atoi(strPtr);

                    printf("Setting section number %i\n", lightNum);

                    if (lightNum < NUM_SECTIONS)
                    {
                        //Updated LED settings
                        calculate_LED_settings(lightNum, givenDuty, frequency, get_current_time());
                    }
                    else
                    {
                        printf("ERROR (SetSection): section %i not valid\n", lightNum);
                        bt_write("ERROR: Section out of range", strlen("ERROR: Section out of range"));
                    }

                    
                }

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
        ledc_set_freq(LEDC_MODE, LED_TIMERS[i], 1000);

        calculate_LED_settings(i, 0, 0, get_current_time());
    }

    //Turn off LED relay
    //turn_off_relay();

}

//Get current time on timer
float get_current_time(void)
{
    uint32_t resolution_hz;
    ESP_ERROR_CHECK(gptimer_get_resolution(gptimer, &resolution_hz));
    uint64_t count;
    ESP_ERROR_CHECK(gptimer_get_raw_count(gptimer, &count));
    float current_time = (double)count / resolution_hz;

    return current_time;
}

//Initializes pins for GPIO
void init_GPIO(void)
{
    //GPIO2 for output to relay
    gpio_config_t io_config_2 = {};
    io_config_2.intr_type = 0;
    io_config_2.mode = GPIO_MODE_OUTPUT;
    io_config_2.pin_bit_mask = 1ULL << 2;
    io_config_2.pull_down_en = 0;
    io_config_2.pull_up_en = 0;
    gpio_config(&io_config_2);

    //GPIO4 for output to relay
    gpio_config_t io_config_4 = {};
    io_config_4.intr_type = 0;
    io_config_4.mode = GPIO_MODE_OUTPUT;
    io_config_4.pin_bit_mask = 1ULL << 4;
    io_config_4.pull_down_en = 0;
    io_config_4.pull_up_en = 0;
    gpio_config(&io_config_4);
}

//Initializes both gptimer (sequence timing) and esptimer (interrupt for relay control)
void init_timers(void)
{
    //GPTIMER
    // Create a timer instance
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));
    // Enable the timer
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    // Start the timer
    ESP_ERROR_CHECK(gptimer_start(gptimer));


    //ESPTIMER
    const esp_timer_create_args_t oneshot_timer_args = {
        .callback = &timer_callback_end_signal,
        .name = "oneshot-timer"
    };
    ESP_ERROR_CHECK(esp_timer_create(&oneshot_timer_args, &oneshot_timer));

}

//Callback function when esptimer ends to cancel the gpio signal that was set high
void timer_callback_end_signal(void* arg)
{
    //Turns off gpio that was previously turned on 
    gpio_set_level(activeRelayGPIO, 0);
}

//Sends signal to turn on relay (GPIO2)
void turn_on_relay(void)
{

    relayState = ON;

    gpio_set_level(2, 1);

    activeRelayGPIO = 2;

    //Start timer to shut off after short period (50 ms)
    esp_timer_stop(oneshot_timer);
    ESP_ERROR_CHECK(esp_timer_start_once(oneshot_timer, 50000));

    printf("Turning on relay\n");
}

//Sends signal to turn off relay (GPIO4)
void turn_off_relay(void)
{
    relayState = OFF;

    gpio_set_level(4, 1);

    activeRelayGPIO = 4;

    //Start timer to shut off after short period (50 ms)
    esp_timer_stop(oneshot_timer);
    ESP_ERROR_CHECK(esp_timer_start_once(oneshot_timer, 50000));

    printf("Turning off relay\n");
}

//Run all initialize functions
void initialize(void)
{
    init_GPIO();
    init_leds();
    init_state();
    init_timers();
}

