/*******************************************************************************
  * @file    main.c
  * @author  Ashraf Suyyagh
	* @version V1.2.0
  * @date    17-January-2016
  * @brief   This file demonstrates flasing one LED at an interval of one second
	*          RTX based using CMSIS-RTOS 
  ******************************************************************************
  */

#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "RTE_Components.h"             // Component selection
#include "main.h"


extern void initializeLED_IO			(void);
extern void start_Thread_LED			(void);
extern void Thread_LED(void const *argument);
extern osThreadId tid_Thread_LED;

// Mutex for temp and angle
osMutexId temp_mutex;
osMutexId angle_mutex;
osMutexDef(temp_mutex);
osMutexDef(angle_mutex);


void Timer_Callback1  (void const *arg);
void Timer_Callback2  (void const *arg);

osTimerDef (Timer1, Timer_Callback1);
osTimerDef (Timer2, Timer_Callback2);

int alarm_counter;
float coeff[Order];
float display;

/**
	These lines are mandatory to make CMSIS-RTOS RTX work with te new Cube HAL
*/
#ifdef RTE_CMSIS_RTOS_RTX
extern uint32_t os_time;

uint32_t HAL_GetTick(void) { 
  return os_time; 
}
#endif


/**
  * System Clock Configuration
  */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the
     device is clocked below the maximum system frequency (see datasheet). */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 |
                                RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}





//timer callback for 7-segment
void Timer_Callback1  (void const *arg){
    printf("callback 1 beginning\n");
	if(display_mode != -1) {
		//Increment the alarm counter, or reset it
		if(alarm_counter % 1000 == 0)
            alarm_counter = 0;
        else
            alarm_counter++;
		
		//Update the segment display when alarm is off
		if (alarm_on == 0) {
			update_7seg_disp(fabsf(display));
		}
		//triggers alarm "on" portion
		else if(alarm_counter > 500){
				//grabs all select lines at once
			HAL_GPIO_WritePin(GPIOB , GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_SET);
				//turns segment display on
			HAL_GPIO_WritePin (GPIOD , GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 |GPIO_PIN_4 |GPIO_PIN_5 |GPIO_PIN_6 |GPIO_PIN_7 , GPIO_PIN_SET);
		}
		//triggers alarm "off" portion
		else{
				//grabs all select lines at once
			HAL_GPIO_WritePin(GPIOB , GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_RESET);
				//turns segment display on
			HAL_GPIO_WritePin (GPIOD , GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 |GPIO_PIN_4 |GPIO_PIN_5 |GPIO_PIN_6 |GPIO_PIN_7 , GPIO_PIN_RESET);
		}
            
	}
}	
//callback for keypad
void Timer_Callback2  (void const *arg){
    determine_input(get_key());
    printf("display mode is %i \n", display_mode);
    printf("display value is: %f \n",display);
   // update_7seg_disp(absolute(display));
}

/**
  * Main function
  */
int main (void) {
	
    osKernelInitialize();                     /* initialize CMSIS-RTOS          */

    HAL_Init();                               /* Initialize the HAL Library     */

    SystemClock_Config();                     /* Configure the System Clock     */
    __HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_ADC1_CLK_ENABLE();

	/* User codes goes here*/
  initializeLED_IO();                       /* Initialize LED GPIO Buttons    */
  start_Thread_LED();                       /* Create LED thread              */
    //config the stuff
    generate_coeffecients(Order);
    
    config_7seg();
 
  //  printf("done config 7-seg\n");
    
    config_interrupt();
   // printf("done config interrupt\n");
        
    config_accelerometer();
  //  printf("done config accelerometer\n");
        
    config_adc();    
   // printf("done config adc\n");
    
    config_timer();
    //printf("done config timer\n");
 
	  printf("GPIO initialized\n");
		display_mode = 0;


	// create mutex 
	temp_mutex = osMutexCreate(osMutex(temp_mutex));
	angle_mutex = osMutexCreate(osMutex(angle_mutex));
    
	// thread functions 
	start_Thread_polling_temp();	
	start_Thread_angle();
	start_Thread_temp();

	//starts timers
  osTimerStart (osTimerCreate (osTimer(Timer1), osTimerPeriodic, (void *) 0),2);
  osTimerStart (osTimerCreate (osTimer(Timer2), osTimerPeriodic, (void *) 0), 1);

    /* User codes ends here*/
  
	osKernelStart();                          /* start thread execution         */

}
