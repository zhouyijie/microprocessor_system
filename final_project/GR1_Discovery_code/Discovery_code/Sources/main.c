/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program subroutine
	* Author						 : Ashraf Suyyagh
	* Version            : 1.0.0
	* Date							 : January 14th, 2016
  ******************************************************************************
  */
	
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "supporting_functions.h"
//#include "stm32f4xx_it.c"
//#include "accelerometer.h"
#include "lis3dsh.h"
#include "main.h"
#include "math.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define M_PI 3.14159265358979323846
#define 	USARTx_RX_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOA_CLK_ENABLE()
 
#define 	USARTx_TX_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOA_CLK_ENABLE()

/* Private variables ---------------------------------------------------------*/

//global constants
TIM_HandleTypeDef TIM3_Handle;
LIS3DSH_InitTypeDef LIS3DSH_Initialize;
LIS3DSH_DRYInterruptConfigTypeDef LIS3DSH_Init_Config;

DAC_HandleTypeDef    hdac;
DAC_ChannelConfTypeDef sConfig;

int global_counter;	
float acceleration_normalized[3];
float accel_temp[3];
int i;
float pitch;
float roll;

float UE_X = 50.2312;
float UE_Y = 19.2176;
float UE_Z = 55.2099;
float CX = 1.0418;
float CY = 1.0024;
float CZ = 0.9848;

int pina;

//FIR functions
#define Length 50
#define Order 5
float coeff[] = {.1,.15,.5,.15,.1};
float acceleration_reading_x[Length];		
float acceleration_reading_y[Length];		
float acceleration_reading_z[Length];

//initialize keypad state at 0, which would be idle
int keystate = 0;
int input;
int new_digit;
int input_flag;
int threshold_set_flag;

float calibration_param_matrix[4][3] = {
	{0.000975054748, -0.0000182180993, -0.0000000305652774},
	{-0.0000307680088, 0.00100024108, -0.00000438213129},
	{-0.00000617881222, -0.00000113744427, 0.000963388786},
	{0.00214820029, 0.00162097280, -0.0131357405}
};

GPIO_InitTypeDef GPIO_Init;
int ext_io_flag = 0;
int TIM3_flag = 0;
int global_counter;
int TIM3_counter;
float threshold;
int input_value;

UART_HandleTypeDef huart2;
__IO ITStatus UartReady = RESET;


/* Buffer used for transmission */
uint8_t aTxBuffer[12] = {0,0,0,0,0,0,0,0,0,0,0,0};

/* Buffer used for reception */
uint8_t aRxBuffer[4] = {0,0,0,0};



#define TOLERANCE 4.0

void config_uart (void)
{
	
	__HAL_RCC_USART2_CLK_ENABLE();

	/* Enable interface clock */ 
	USARTx_RX_GPIO_CLK_ENABLE();
	USARTx_TX_GPIO_CLK_ENABLE();
	
	
	 
	
	/* TX pin configuration */
  GPIO_Init.Pin = GPIO_PIN_2;
	GPIO_Init.Alternate = GPIO_AF7_USART2;
	GPIO_Init.Mode = GPIO_MODE_AF_PP;
	GPIO_Init.Pull = GPIO_NOPULL;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
  HAL_GPIO_Init(GPIOA, &GPIO_Init);
	
	/* RX pin configuration */
	GPIO_Init.Pin = GPIO_PIN_3;
	GPIO_Init.Alternate = GPIO_AF7_USART2;
	
  HAL_GPIO_Init(GPIOA, &GPIO_Init);
	
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&huart2);
}

void config_accelerometer(void)
{
  //Configure the accelerometer initialization struct
  LIS3DSH_Initialize.Power_Mode_Output_DataRate = LIS3DSH_DATARATE_25;        /* Ppower down or /active mode with output data rate 3.125 / 6.25 / 12.5 / 25 / 50 / 100 / 400 / 800 / 1600 HZ */
  LIS3DSH_Initialize.Axes_Enable = LIS3DSH_XYZ_ENABLE;                        /* Axes enable */
  LIS3DSH_Initialize.Continous_Update = LIS3DSH_ContinousUpdate_Disabled;			/* Block or update Low/High registers of data until all data is read */
  LIS3DSH_Initialize.AA_Filter_BW = LIS3DSH_AA_BW_50;													/* Choose anti-aliasing filter BW 800 / 400 / 200 / 50 Hz*/
  LIS3DSH_Initialize.Full_Scale = LIS3DSH_FULLSCALE_2;                        /* Full scale 2 / 4 / 6 / 8 / 16 g */
  LIS3DSH_Initialize.Self_Test = LIS3DSH_SELFTEST_NORMAL;                     /* Self test */
	
  //Configure the accelerometer interrupt struct
  LIS3DSH_Init_Config.Dataready_Interrupt = LIS3DSH_DATA_READY_INTERRUPT_ENABLED;     /* Enable/Disable data ready interrupt */
  LIS3DSH_Init_Config.Interrupt_signal = LIS3DSH_ACTIVE_HIGH_INTERRUPT_SIGNAL;         /* Interrupt Signal Active Low / Active High */
  LIS3DSH_Init_Config.Interrupt_type = LIS3DSH_INTERRUPT_REQUEST_PULSED;							/* Interrupt type as latched or pulsed */
	
  //Enable the interrupt line
  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 1);
	
  //Initialize the accelerometer
  LIS3DSH_Init(&LIS3DSH_Initialize);
	
  //Setting the interrupt configuration of the accelerometer
  LIS3DSH_DataReadyInterruptConfig(&LIS3DSH_Init_Config);		
}

void config_interrupt(void)
{
	//Configure GPIOE for the interrupt line
	GPIO_Init.Pin = GPIO_PIN_0;
	GPIO_Init.Mode = GPIO_MODE_IT_RISING;
	GPIO_Init.Speed = GPIO_SPEED_FREQ_HIGH;
	GPIO_Init.Pull = GPIO_NOPULL ; 
	HAL_GPIO_Init(GPIOE, &GPIO_Init);
}

void config_timer() 
{
	__TIM3_CLK_ENABLE();
	TIM3_Handle.Instance = TIM3;
	TIM3_Handle.Init.Period= 400; //401
	TIM3_Handle.Init.Prescaler = 100; //101
	TIM3_Handle.Init.CounterMode = TIM_COUNTERMODE_UP; 
	TIM3_Handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1; 
	TIM3_Handle.Init.RepetitionCounter = 0;	
	
	HAL_TIM_Base_Init(&TIM3_Handle);
	HAL_TIM_Base_Start_IT(&TIM3_Handle);
	
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_0);
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
	HAL_NVIC_SetPriority(TIM3_IRQn, 0, 1);
}

//supporting functions----------------------------------------


void get_acceleration(void)
{	

	//Get the accelerometer readings
	LIS3DSH_ReadACC(accel_temp);
	
	acceleration_normalized[0] = (accel_temp[0] - UE_X)*CX;
	acceleration_normalized[1] = (accel_temp[1] + UE_Y)*CY;
	acceleration_normalized[2] = (accel_temp[2] - UE_Z)*CZ;
	
	
	
		
	//Print accelerometer readings
	for(i = 0; i < 3; i++)
	{
		//printf("accel: %f,", acceleration_normalized[i]);
	}
	
	//printf("\n");
	//Calculate the roll and pitch angles
	pitch = atan2(acceleration_normalized[0],acceleration_normalized[2]) * 180/ M_PI;
	roll = atan2(acceleration_normalized[1],acceleration_normalized[2]) * 180/ M_PI;
	
	//printf("pitch and roll are %f, %f \n",pitch, roll);
	//reset normalized aceleration
	//acceleration_normalized[0] =0 ;
	//acceleration_normalized[1] =0 ;
	//acceleration_normalized[2] =0 ;
}



int get_key(void){

	

	//idle
	if(keystate == 0){
		global_counter = 0;
		if(pina){
			keystate++;
			}
	}
	else if(keystate == 1){
		global_counter = 0;
		if (global_counter % 1000 == 0){
			
			if(pina){
				keystate++;
			}
		}
	}
	else if(keystate == 2){
	 	if(!pina){
			keystate++;

		}
	}
	else if(keystate == 3){
		global_counter = 0;
		if(global_counter % 1000 == 0){
			keystate = 0;
			return 1;
		}
	}
	else{
		keystate = 0;
	}
	
	return 0;
}

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config	(void);
	
int main(void)
{	
	int a;
	float tiltangle;
	float test;
	uint32_t RxData;
	
	
  /* MCU Configuration----------------------------------------------------------*/
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();
	
  /* Initialize all configured peripherals */
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_DAC_CLK_ENABLE();
	
	GPIO_Init.Pin = GPIO_PIN_0;
	GPIO_Init.Mode = GPIO_MODE_INPUT;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	//may need GPIO_PULLUP if the signal changes too much
	GPIO_Init.Pull = GPIO_NOPULL; 
	HAL_GPIO_Init(GPIOA, &GPIO_Init);
	
	
	config_accelerometer();
	config_interrupt();
	config_timer();
	HAL_Init();
	config_uart();
	
	
	
	hdac.Instance = DAC;
	HAL_DAC_Init(&hdac);
	
	sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;  
	HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1);
	
	
	//DAC_ENABLE is called in DAC_START
	//__HAL_DAC_ENABLE(&hdac, DAC_CHANNEL_1); 
	
	HAL_DAC_Start(&hdac, DAC1_CHANNEL_1);
	

	
	while (1){
		
		
	

		get_acceleration();
		pina = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0);
		a = get_key();
		if (a){
			
			//printf("press...\n");
		}
		
		
		memcpy(aTxBuffer,&acceleration_normalized[0],4);
		memcpy((aTxBuffer+4),&acceleration_normalized[1],4);
		memcpy((aTxBuffer+8),&acceleration_normalized[2],4);
//		memcpy(&test,aTxBuffer,4);
//		RxData = (uint32_t) test;
//		printf("data is %d\n",RxData);
		
		
		if (HAL_UART_Transmit(&huart2, aTxBuffer, 12,5000) != HAL_OK ){
			//printf("not transmit x...\n");
		}
		
		if (HAL_UART_Receive(&huart2, aRxBuffer, 4,5000) != HAL_OK){
			//printf("not receive first...\n");
		}
		
		memcpy(&tiltangle,aRxBuffer,4);
		RxData = (uint32_t) tiltangle;
		HAL_DAC_SetValue(&hdac, DAC1_CHANNEL_1, DAC_ALIGN_12B_R, RxData);

		
		global_counter++;
		TIM3_flag = 0;
		//TIM3_counter;
		//reset counter
		if(global_counter==1000) 
			global_counter = 0;
		}
		
	
	
}

/** System Clock Configuration*/
void SystemClock_Config(void){

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  __PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType 	= RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState 			 	= RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState 		= RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource 	= RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM 				= 8;
  RCC_OscInitStruct.PLL.PLLN 				= 336;
  RCC_OscInitStruct.PLL.PLLP 				= RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ 				= 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK){Error_Handler(RCC_CONFIG_FAIL);};

  RCC_ClkInitStruct.ClockType 			= RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource 		= RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider 	= RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider 	= RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider 	= RCC_HCLK_DIV2;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5)!= HAL_OK){Error_Handler(RCC_CONFIG_FAIL);};
	
	/*Configures SysTick to provide 1ms interval interrupts. SysTick is already 
	  configured inside HAL_Init, I don't kow why the CubeMX generates this call again*/
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

	/* This function sets the source clock for the internal SysTick Timer to be the maximum,
	   in our case, HCLK is now 168MHz*/
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	ext_io_flag = 1;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	TIM3_counter ++;
	TIM3_flag = 1;
}
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart2)
{
  /* Set transmission flag: transfer complete */
  UartReady = SET;
  
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart2)
{
  /* Set transmission flag: transfer complete */
  UartReady = SET;
  
}


#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line){
}
#endif

