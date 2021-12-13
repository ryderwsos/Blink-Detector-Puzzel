#include <stdbool.h> // booleans, i.e. true and false
#include <stdio.h>   // sprintf() function
#include <stdlib.h>  // srand() and random() functions
#include "ece198.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main();
bool startGame(int start_length, int final_length);
int generateGame(int *gameValue, int length);
void blinkLED(int *gameValue, int length);
void DisplaySensor(uint16_t pin);
bool playGame(int *gameValue, int length);
int timeBlinked();

int main(void)
{
    //initializing all the pins
    HAL_Init();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    InitializePin(GPIOA, GPIO_PIN_8 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_12, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL, 0);
    InitializePin(GPIOA, GPIO_PIN_10, GPIO_MODE_INPUT, GPIO_PULLDOWN, 0);
    InitializePin(GPIOC, GPIO_PIN_13, GPIO_MODE_INPUT, GPIO_PULLUP, 0);

    //serial setups
    SerialSetup(9600);
    SerialPuts("\r\n\n");
    const int start_length = 2;
    const int final_length = 6;
	
    //wait for user to press the blue button to start
    for (int i = 0; i<3;++i){
        while (true){
            while(!HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)){
            }
            while(HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13)){
            }
            break;
        }
    }
    srand(HAL_GetTick());
    //start the game
    while(true){
        HAL_Delay(500);
        bool pass = startGame(start_length, final_length);
        if (pass){
            break;
        }
    }
    return 0;
}

//this function starts the game
bool startGame(int start_length, int final_length){
    bool pass = true;
    for(int i = start_length; i <= final_length; i++){                  //loops through multiple games
        int array[i]; 
        generateGame(array, i);                                         //generates array of the blink sequence
        blinkLED(array, i);                                             //blinks the led based on the array
        HAL_Delay(250);                                     
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);            //lights up the pin when it waits for user input
        pass = playGame(array, i);                                      
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);          //turns off the play game light
        if (!pass){
            break;                                                      //restarts the game if the user does not pass a round
        }
    }
    if (pass){                                                          //blinks the pass led rapidly if the game is won
    uint16_t pins[4] = {GPIO_PIN_12,GPIO_PIN_8,GPIO_PIN_5,GPIO_PIN_6};
        for (int k = 0; k < 40; ++k){                                  
            HAL_Delay(125);
            HAL_GPIO_WritePin(GPIOA,pins[k%4], GPIO_PIN_SET);
            HAL_Delay(125);
            HAL_GPIO_WritePin(GPIOA,pins[k%4],GPIO_PIN_RESET);
        }
    }
    return pass;
}

//randomly generates the sequence array
int generateGame(int *gameValue, int length){
    for(int i = 0; i < length; i++){
        gameValue[i] = rand()%2;
    }
    return *gameValue;
}

//blink the blue led based on generated sequence
void blinkLED(int *gameValue, int length){
    for(int i = 0; i < length; ++i ){
        HAL_Delay(500);
        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8, GPIO_PIN_SET);
        if (gameValue[i] == 1){                                         //if the sequence is 1, its a long blink
            HAL_Delay(2000);
            HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8, GPIO_PIN_RESET);
        }
        else{
            HAL_Delay(750);
            HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8, GPIO_PIN_RESET);        //if the sequence is 2, its a short blink
        }
    }
}

//function to take in user's input
bool playGame(int *gameValue, int length){
    int array[length];
    bool pass = true;
    for (int i = 0; i < length; ++i){                                   //loops through the amount of input 
        int time = 0;
        while (time == 0) {                                             //makes sure that the blink is over a threshold of ms blinked
            time = timeBlinked();
        }
        if (time >= 1400){                                              //inputs 1 to an array if blink was long
            array[i] = 1;
        }
        else if(time > 400){                                            //inputs 0 to an array if blink was short
            array[i] = 0;
        }
    }
    for (int i = 0; i < length; ++i){                                   //match if the squences blinked was right
        if (array[i] != gameValue[i]){
            pass = false;
        }
    }

    if (pass){                                                          //if the round is passed, light up green led
        HAL_Delay(100);
        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5, GPIO_PIN_SET);
        HAL_Delay(3000);
        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_5, GPIO_PIN_RESET);
    }
    else{                                                               //if the round is loss, light up red led
        HAL_Delay(100);
        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6, GPIO_PIN_SET);
        HAL_Delay(3000);
        HAL_GPIO_WritePin(GPIOA,GPIO_PIN_6, GPIO_PIN_RESET);
    }
    return pass;
}

//calculates the amount of time pressed
int timeBlinked(){                                                    
    uint32_t startTime = 0;   
    while(!HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_10)){
    }
    if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10)){                           //takes in the tick that the player started blinking
        startTime = HAL_GetTick();
    }
    while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10)){                        //makes sure that the blink isn't a mis-blink(pass a threshold)
        if ((HAL_GetTick() - startTime) >= 400){
            break;
        }
    }
    if(HAL_GetTick()-startTime < 400) { 
        return 0;
    }

    while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10)){                        //waits for the user to finish blinking
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
    }
    uint32_t finishTime = HAL_GetTick();
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8,GPIO_PIN_RESET);
    int time = finishTime - startTime;
    return time;
}

void SysTick_Handler(void){
    HAL_IncTick();
}
