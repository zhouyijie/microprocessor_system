/*******************************************************************************
  * @file    config_files.c
  * @author  Alex Richardson
	* @version V1.2.0
  * @date    17-January-2016
  * @brief   This file sets up the configs for the project
	*          RTX based using CMSIS-RTOS 
  ******************************************************************************
  */

//#include "config_files.h"
#include "main.h"

//For ADC
ADC_InitTypeDef ADC_init;
ADC_ChannelConfTypeDef ADC_ChannelStruct;
ADC_HandleTypeDef ADC1_handler;

GPIO_InitTypeDef GPIO_Init;
TIM_HandleTypeDef TIM3_handler;

LIS3DSH_InitTypeDef LIS3DSH_Initialize;
LIS3DSH_DRYInterruptConfigTypeDef LIS3DSH_Init_Config;

void config_7seg (void){
//	//Push button
//	GPIO_Init.Pin = GPIO_PIN_0;
//	GPIO_Init.Mode = GPIO_MODE_INPUT;
//	GPIO_Init.Speed = GPIO_SPEED_FAST;
//	//may need GPIO_PULLUP if the signal changes too much
//	GPIO_Init.Pull = GPIO_NOPULL; 
//	HAL_GPIO_Init(GPIOA, &GPIO_Init);
    //seven-segment setup
	GPIO_Init.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	GPIO_Init.Pull = GPIO_NOPULL; 
	HAL_GPIO_Init(GPIOD, &GPIO_Init);
	//Select lines
	GPIO_Init.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	GPIO_Init.Pull = GPIO_NOPULL; 
	HAL_GPIO_Init(GPIOB, &GPIO_Init);
}

void config_interrupt(void){
//Configure GPIOE for the interrupt line
     GPIO_Init.Pin = GPIO_PIN_0;
     GPIO_Init.Mode = GPIO_MODE_IT_RISING;
     GPIO_Init.Speed = GPIO_SPEED_FREQ_HIGH;
     GPIO_Init.Pull = GPIO_NOPULL ; 
     HAL_GPIO_Init(GPIOE, &GPIO_Init);
}

void config_timer(void){

	//1kHz
	__TIM3_CLK_ENABLE();
	//configure TIM3_handler
	TIM3_handler.Instance = TIM3;
	TIM3_handler.Init.Period= 420; //401
	TIM3_handler.Init.Prescaler = 100; //101
	TIM3_handler.Init.CounterMode = TIM_COUNTERMODE_UP; 
	TIM3_handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1; 
	TIM3_handler.Init.RepetitionCounter = 0;	
	
	
	//start with interrupt
	HAL_TIM_Base_Init(&TIM3_handler);
	HAL_TIM_Base_Start_IT(&TIM3_handler);
	
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
	HAL_NVIC_SetPriority(TIM3_IRQn, 0, 1);
}

void config_adc(void){
    ////ADC setup
	//printf("\nADC Handling has begun");
	//Set ADC handler to ADC1
	ADC1_handler.Instance = ADC1 ;
	//Configure ADC1 parameters
	ADC_init.DataAlign = ADC_DATAALIGN_RIGHT;
	ADC_init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV4; 
	ADC_init.Resolution = ADC_RESOLUTION_12B; 
	ADC_init.ContinuousConvMode = ENABLE; 
	ADC_init.DiscontinuousConvMode = DISABLE; 
	ADC_init.NbrOfConversion = 1; 
	ADC_init.ScanConvMode = DISABLE; 
	ADC_init.DMAContinuousRequests = DISABLE; 
	ADC1_handler.Init = ADC_init;

	if (HAL_ADC_Init(&ADC1_handler) != HAL_OK){
		
	}
	//ADC channel config
	ADC_ChannelStruct.Channel = ADC_CHANNEL_16;
	ADC_ChannelStruct.SamplingTime = ADC_SAMPLETIME_480CYCLES; 
	ADC_ChannelStruct.Rank = 1;
	ADC_ChannelStruct.Offset = 0;
	
	//Send error if this doesnt work, ie channel config is screwy
	if (HAL_ADC_ConfigChannel(&ADC1_handler, &ADC_ChannelStruct) != HAL_OK )
	{
		//stack exchange says this is a valid way to do this. wil need to test to make sure
		//assert_failed(__FILE__,__LINE__);
	}	
	if (HAL_ADC_Start(&ADC1_handler) != HAL_OK) {
		
	}
}

void config_accelerometer(void){
  //Configure the accelerometer initialization struct
  LIS3DSH_Initialize.Power_Mode_Output_DataRate = LIS3DSH_DATARATE_25;        /* Ppower down or /active mode with output data rate 3.125 / 6.25 / 12.5 / 25 / 50 / 100 / 400 / 800 / 1600 HZ */
  LIS3DSH_Initialize.Axes_Enable = LIS3DSH_XYZ_ENABLE;                        /* Axes enable */
  LIS3DSH_Initialize.Continous_Update = LIS3DSH_ContinousUpdate_Disabled; /* Block or update Low/High registers of data until all data is read */
  LIS3DSH_Initialize.AA_Filter_BW = LIS3DSH_AA_BW_50; /* Choose anti-aliasing filter BW 800 / 400 / 200 / 50 Hz*/
  LIS3DSH_Initialize.Full_Scale = LIS3DSH_FULLSCALE_2;                        /* Full scale 2 / 4 / 6 / 8 / 16 g */
  LIS3DSH_Initialize.Self_Test = LIS3DSH_SELFTEST_NORMAL;                     /* Self test */
  //Configure the accelerometer interrupt struct
  LIS3DSH_Init_Config.Dataready_Interrupt = LIS3DSH_DATA_READY_INTERRUPT_ENABLED;     /* Enable/Disable data ready interrupt */
  LIS3DSH_Init_Config.Interrupt_signal = LIS3DSH_ACTIVE_HIGH_INTERRUPT_SIGNAL;         /* Interrupt Signal Active Low / Active High */
  LIS3DSH_Init_Config.Interrupt_type = LIS3DSH_INTERRUPT_REQUEST_PULSED; /* Interrupt type as latched or pulsed */
  //Enable the interrupt line
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 1);
  //Initialize the accelerometer
  LIS3DSH_Init(&LIS3DSH_Initialize);
  //Setting the interrupt configuration of the accelerometer
  LIS3DSH_DataReadyInterruptConfig(&LIS3DSH_Init_Config); 
}
