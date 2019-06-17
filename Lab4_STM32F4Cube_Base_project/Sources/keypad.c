/*******************************************************************************
  * @file    config_files.c
  * @author  Alex Richardson
	* @version V1.2.0
  * @date    17-January-2016
  * @brief   This file allows keypad use for project
	*          RTX based using CMSIS-RTOS 
  ******************************************************************************
  */
	
#include "main.h"

int input, new_digit;
int keypad[16] = {1,2,3,4,5,6,7,8,9,-1,1,-2};
int keystate = 0;
int p_or_r = 0;
int input_flag = 0;

void determine_input (int key){
	
	//pound sign pressed, swich between roll and pitch
	if(key == -1){
		 input_flag = 1;
		 display_mode = 1;
		//alternates between roll and pitch
		p_or_r = absolute( p_or_r -1);
		
	}
	else if(key == -2){
		display_mode = 0;
		input_flag = 1;
	}
	else if(key < 10 && key > 0){
		display_mode = 1;

	}
	
}


int get_key(void){

     //note: logic is reversed, ie a 0 is a button push
     int column;
     int row;
     //find column first
     //first 4 pins set to out
     GPIO_Init.Pin = GPIO_PIN_4| GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
     GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
     GPIO_Init.Speed = GPIO_SPEED_FAST;
     GPIO_Init.Pull = GPIO_NOPULL; 
     HAL_GPIO_Init(GPIOC, &GPIO_Init);
     //last 4 pins set to in
     GPIO_Init.Pin = GPIO_PIN_15 | GPIO_PIN_14 | GPIO_PIN_13 | GPIO_PIN_12;
     GPIO_Init.Mode = GPIO_MODE_INPUT;
     GPIO_Init.Speed = GPIO_SPEED_FAST;
     GPIO_Init.Pull = GPIO_PULLUP; 
     HAL_GPIO_Init(GPIOE, &GPIO_Init);
     // printf("%d",HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_15));
     // printf("%d",HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14));
     // printf("%d",HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_13));
     // printf("%d\n",HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12));

     //if ( HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_15) == GPIO_PIN_RESET ) {
     //column = 0;
     //}
     if ( HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14) == GPIO_PIN_RESET && HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_13) == GPIO_PIN_SET && HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12) == GPIO_PIN_SET ) {
        column = 0;
     }
     else if ( HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_13) == GPIO_PIN_RESET && HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12) == GPIO_PIN_SET && HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14) == GPIO_PIN_SET) {
        column = 1;
     }
     else if ( HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12) == GPIO_PIN_RESET ) {
        column = 2;
     }
     else {
        column = 4;
     }

     //find row
     //first 4 pins of the alphanumeric set to INPUT
     GPIO_Init.Pin = GPIO_PIN_4| GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
     GPIO_Init.Mode = GPIO_MODE_INPUT;
     GPIO_Init.Speed = GPIO_SPEED_FAST;
     GPIO_Init.Pull = GPIO_PULLUP; 
     HAL_GPIO_Init(GPIOC, &GPIO_Init);
     //last 4 pins of the alphanumeric set to OUTPUT
     GPIO_Init.Pin = GPIO_PIN_15 | GPIO_PIN_14 | GPIO_PIN_13 | GPIO_PIN_12;
     GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
     GPIO_Init.Speed = GPIO_SPEED_FAST;
     GPIO_Init.Pull = GPIO_NOPULL; 
     HAL_GPIO_Init(GPIOE, &GPIO_Init);
     // printf("%d",HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_4));
     // printf("%d",HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_5));
     // printf("%d",HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6));
     // printf("%d\n",HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_7));
     if ( HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6) == GPIO_PIN_RESET && HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_7) == GPIO_PIN_RESET) {
        row = 0;
     }
     else if ( HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_5) == GPIO_PIN_RESET && HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_7) == GPIO_PIN_RESET ) {
        row = 1;
     }
     else if ( HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_4) == GPIO_PIN_RESET && HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_7) == GPIO_PIN_RESET ) {
        row = 2;
     }
     else if ( HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_7) == GPIO_PIN_RESET ) {
        row = 3;
     }
     else {
        row = 4;
     }



      //printf("column is %i and ", column);
      //printf("row is %i\n", row);

     //idle
     if(keystate == 0){
         if(row <4 && column < 4){
					 new_digit = keypad[3*row+column];
					 printf("new_digit is %i \n", new_digit);
					 keystate++;
				 }
				 //out of bounds, do not interpret
				 else{
					 return 10;
				 }
     }//keystate = idle
     
     // debouncing, as suggested by the document
     else if(keystate == 1){
         //if its time to take a key value then shift previous number left by 1
         //printf("Curent input is %i\n", input);
         keystate++;
     }
     //key has been released
     else if(keystate == 2){
         if(row == 4 && column == 4){
         keystate=0;
         }
     }
     else{
        keystate = 0;
     }
     //printf("input is %i", input);
     return new_digit;
}//end get_key()
