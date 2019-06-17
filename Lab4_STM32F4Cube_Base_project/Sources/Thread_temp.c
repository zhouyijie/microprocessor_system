/* Includes ------------------------------------------------------------------*/

#include "main.h"

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
int display_temp_counter;
/* USER CODE END PFP */
#define overheat_c	60.0
#define overheat_f 60.0
/* USER CODE BEGIN 0 */
extern float shown_temp;

void Thread_temp (void const *argument);

//create RTOS thread for temperature

osThreadId tid_Thread_temp;                              // thread id
osThreadDef(Thread_temp, osPriorityNormal, 1, 0);	

int start_Thread_temp (void) {

     
     printf("start thread temp\n");
  tid_Thread_temp = osThreadCreate(osThread(Thread_temp ), NULL); // Start Temp Thread
  if (!tid_Thread_temp)
      return(-1); 
  return(0);
}

/*
//Set the alarm to turn it off
void alarming_conclusion(){
	if(!alarm_on){
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15, GPIO_PIN_RESET);
		return;
	}
	else{
		//first led
		if(loop < alarm_time){
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);			
		}
		//second led. Using else if instead of anothe if statement b/c it allows for potentially less hardware
		else if(loop < 2*alarm_time){
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);			
		}
		//third led
		else if(loop < 3*alarm_time){
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);			
		}
		//fourth led. May need to change to "else if" later
		else{
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, GPIO_PIN_SET);			
		}
	}
	
}

*/

void Thread_temp (void const *argument) {
	//display_temp_counter = 0;     

	while(1){       
		osSignalWait(1, osWaitForever);
		osMutexWait(temp_mutex, osWaitForever);
		//printf("Thread_temp while loop started\n");
		
		//Update the display value of the temp every half-second
		if (display_mode == 0) {
			if (display_temp_counter % 500 == 0) {
				display = shown_temp;	
				display_temp_counter = 0;
        printf("shown temp is: %f", shown_temp);
			}
		}		
		osMutexRelease(temp_mutex);
		osSignalClear(tid_Thread_temp, 1);
	}//end while
}//end main

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	//printf("\ntimer last word?\n");
	//Increments display_temp_counter when in temp display mode
	if(display_mode == 0)
     display_temp_counter++; 
	
	osSignalSet(tid_Thread_temp, 1);
}