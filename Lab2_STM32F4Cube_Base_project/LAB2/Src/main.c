/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
//Both of the following are done in the main instead
//#include "adc.h"
//#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_adc.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
	ADC_InitTypeDef ADC_init;
	ADC_ChannelConfTypeDef ADC_ChannelStruct;
	GPIO_InitTypeDef GPIO_init;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

int loop; //used for LED and temp timing. It will be looped in the while and reset after 4x the alarm_period (since there are 4 LEDs)
int alarm_on;
/* USER CODE END PFP */
#define alarm_time	500
#define read_freq 10
#define overheat_c	60.0
#define overheat_f 60.0

/* USER CODE BEGIN 0 */
ADC_HandleTypeDef ADC1_handler;

float mean(float* input, int length){
    
	int i;
	float sum = 0;
	for(i = 0; i < length; i++){
		sum += input[i];
	}
	return (sum / length);
}

int FIR_C(float* InputArray, float* OutputArray, float* coeff, int Length, int Order)
{
	int n;
	int z;
	float weighted_total;

	//apply the filter
	for(n = Order; n< Length; n++){
		//calculate n
		weighted_total = 0;
		//implement the weighting factor by the subset of the data array dicated by the order number
		for( z=0; z< Order; z++){
			weighted_total += coeff[z] * InputArray[n- Order + z];
		}
		*(OutputArray++) = weighted_total;
	}
	
	return 0;
}

//Takes ADC sensor data and converts to Celsius
float sensor_data(int pina){
	float temp;
	float in;
	float voltage_equiv;
	
	in = HAL_ADC_GetValue(&ADC1_handler);
	voltage_equiv = (2.93*in)/4096;
	
	//Reference should be around .76V @ 25C. Temp sense slope is around 25mV/C 
	temp = (((voltage_equiv - 0.76f) / 0.0025f) + 25.0f);

	//C to F: C*9/5 + 32. Only occurs when pin_A is selected
	if(pina){
		temp = temp*9/5 + 32;
	}
	return temp;
}

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

//How we configure 7-segment display:
//SEGMENT A - GPIO_PIN_0
//SEGMENT B - GPIO_PIN_1
//SEGMENT C - GPIO_PIN_2
//SEGMENT D - GPIO_PIN_3
//SEGMENT E - GPIO_PIN_4
//SEGMENT F - GPIO_PIN_5
//SEGMENT G - GPIO_PIN_6
//SEGMENT Decimal Point - GPIO_PIN_7
void display_num(int num){
	//clear before displaying
	HAL_GPIO_WritePin (GPIOE , GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 |GPIO_PIN_4 |GPIO_PIN_5 |GPIO_PIN_6 |GPIO_PIN_7 , GPIO_PIN_RESET);
		//printf("input number is %i \n" , num);
		switch (num) 
	{
		//0. Need pins: 0,1,2,3,4,5
		case 0:
			HAL_GPIO_WritePin (GPIOE, GPIO_PIN_0 |GPIO_PIN_1 |GPIO_PIN_2 |GPIO_PIN_3 |GPIO_PIN_4 | GPIO_PIN_5, GPIO_PIN_SET);
			//printf("display 0 \n");
			break;
		//1. Need pins: 1,2
		case 1:
			HAL_GPIO_WritePin (GPIOE , GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_SET); 
			//printf("display 1 \n");
			break;
		//2. Need pins: 0,1,3,4,6
		case 2:
			HAL_GPIO_WritePin (GPIOE , GPIO_PIN_0 | GPIO_PIN_1 |GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_6 , GPIO_PIN_SET); 
			//printf("display 2 \n");
			break;
		//3. Need pins: 0,1,2,3,6
		case 3:
			HAL_GPIO_WritePin (GPIOE , GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_6, GPIO_PIN_SET);
			//printf("display 3 \n");
			break;
		//4. Need pins: 1,2,5,6
		case 4:
			HAL_GPIO_WritePin (GPIOE , GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_5 | GPIO_PIN_6, GPIO_PIN_SET); 	
			//printf("display 4 \n");
			break;
		//5. Need pins: 0,2,3,5,6
		case 5:
			HAL_GPIO_WritePin (GPIOE ,GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3| GPIO_PIN_5| GPIO_PIN_6, GPIO_PIN_SET); 	
			//printf("display 5 \n");
			break;
		//6. Need pins 0,2,3,4,5,6
		case 6:
			HAL_GPIO_WritePin (GPIOE , GPIO_PIN_0 | GPIO_PIN_2| GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6, GPIO_PIN_SET); 
			//printf("display 6 \n");
			break;
		//7. Need pins 0,1,2
		case 7:
			HAL_GPIO_WritePin (GPIOE ,GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_SET); 			
			//printf("display 7 \n");
			break;
		//8. Need all pins
		case 8:
			HAL_GPIO_WritePin (GPIOE , GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 |GPIO_PIN_4 |GPIO_PIN_5 |GPIO_PIN_6, GPIO_PIN_SET); 		
			//printf("display 8 \n");
			break;
		//9. Need pins 0,1,2,5,6
		case 9:
			HAL_GPIO_WritePin (GPIOE , GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 |GPIO_PIN_5 |GPIO_PIN_6, GPIO_PIN_SET); 	
			//printf("display 9 \n");
			break;
	}
}
void update_7seg_disp(float f){
	int number = f*10;
	int position = 0;
	if(number < 1000){
		while (position != 3){
			HAL_GPIO_WritePin(GPIOC , GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, GPIO_PIN_RESET);
			//Set the value, then set the correct digit to turn on
			switch (position){
				case 0:
					//first digit.
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET) ;
					display_num(number % 10);
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
					//printf("first digit, input number is %i \n" , number);
					
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
					
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);
				break;
				
				case 1:
					//second digit and decimal point
					//decimal point
					//printf("second digit, input number is %i \n" , number);
					
					
					HAL_GPIO_WritePin (GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
					display_num(number % 10);
					HAL_GPIO_WritePin (GPIOE, GPIO_PIN_7, GPIO_PIN_SET) ;
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET) ;				
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);
				break;
				
				case 2:
					//third digit
					//printf("third digit, input number is %i \n" , number);
					
					
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);	
					display_num(number % 10);	
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET) ;
					HAL_GPIO_WritePin (GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);
				break;
				
			}
			//printf("position is %i \n" , position);
			position ++;
			number /= 10;
		}
	}
}
/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
// Instantiate variables used for lab
	float coeff[] = {.1,.15,.5,.15,.1};
	int length = 50;
	int Order = 5;
	float temp[length];
	float temp_filtered[length-Order + 1];
	alarm_on = 0;
	int pina;
	int shown_temp = 0;
	//initialize loop counter to 0
	loop =0;
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

	
  /* USER CODE BEGIN 2 */
//Enable the GPIO & ADC clocks
	//push button
	__HAL_RCC_GPIOA_CLK_ENABLE();
	//7-seg. Used to be GPIOA but it looks like thats needed for pushbutton
	__HAL_RCC_GPIOE_CLK_ENABLE();
	//Select lines
	__HAL_RCC_GPIOC_CLK_ENABLE();
	//LEDs
	__HAL_RCC_GPIOD_CLK_ENABLE();
	//ADC
	__HAL_RCC_ADC1_CLK_ENABLE();
	
	printf("IO setup has begun");
//GPIO setup
	//Push button
	GPIO_init.Pin = GPIO_PIN_0;
	GPIO_init.Mode = GPIO_MODE_INPUT;
	GPIO_init.Speed = GPIO_SPEED_FAST;
	//may need GPIO_PULLUP if the signal changes too much
	GPIO_init.Pull = GPIO_NOPULL; 
	HAL_GPIO_Init(GPIOA, &GPIO_init);
	//seven-segment setup
	GPIO_init.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_init.Speed = GPIO_SPEED_FAST;
	GPIO_init.Pull = GPIO_NOPULL; 
	HAL_GPIO_Init(GPIOE, &GPIO_init);
	//Select lines
	GPIO_init.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
	GPIO_init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_init.Speed = GPIO_SPEED_FAST;
	GPIO_init.Pull = GPIO_NOPULL; 
	HAL_GPIO_Init(GPIOC, &GPIO_init);
	//LEDs
	GPIO_init.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15; 
	GPIO_init.Mode = GPIO_MODE_OUTPUT_PP; 
	GPIO_init.Pull = GPIO_NOPULL;
	GPIO_init.Speed = GPIO_SPEED_FAST; 
	HAL_GPIO_Init(GPIOD, &GPIO_init);
	
//ADC setup
	printf("\nADC Handling has begun");
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
	
	/* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	printf("While loop has begun");
	
	/*
	//test 7 seg 
	while(1){
		printf("set select line\n");
		HAL_GPIO_WritePin(GPIOC , GPIO_PIN_1, GPIO_PIN_SET);
		
		printf("set segment line\n");
		HAL_GPIO_WritePin (GPIOE , GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 |GPIO_PIN_4 |GPIO_PIN_5 , GPIO_PIN_SET);
		
	}
	
	*/
	
	
	
  while (1)
  {
		//if(HAL_ADC_PollForConversion(&ADC1_handler, 1000000) != HAL_OK) {
				//assert_failed(__FILE__,__LINE__); 
		/* USER CODE END WHILE */
		//}
		//reset loop. Put greater than or equals in case of some iteration error
		if(loop >= alarm_time*4){
			loop = 0;
		}
		//get readings for the temp aray to be filtered
		if((loop % read_freq) == 0){
			pina = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0);
			temp[(loop/read_freq)] = sensor_data(pina);
			//printf("read pina value is %i", pina);
		}
		//Display new temp every second or so. Can be changed by changing definition
		if((loop % alarm_time) == 0){
			FIR_C(temp, temp_filtered, coeff, (alarm_time/read_freq), Order);
			shown_temp = mean(temp_filtered, (length-Order+1));
			//printf("temperature value is %i", shown_temp);
			update_7seg_disp(shown_temp);
		}
		//Start alarm if overheating and change alarm status. pina=1 is F, 0 is C
		if(pina && (shown_temp > overheat_f)){
				alarm_on = 1;
				alarming_conclusion();
			
		}
		else if(!pina &&(shown_temp > overheat_c) ){
				alarm_on = 1;
				alarming_conclusion();
		}
		else{
			alarm_on =0;
			alarming_conclusion();
		}
		//update display and update loop. Might want to update only when we calculate a new value

		loop++;
	}
  /* USER CODE BEGIN 3 */

  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  __PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
                              |RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  // User can add his own implementation to report the file name and line number,
  printf("Wrong parameters value: file %s on line %d\r\n", file, line);
  // USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
