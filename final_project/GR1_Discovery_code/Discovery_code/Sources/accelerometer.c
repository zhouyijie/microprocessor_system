
/*accelerometer is a bit of a misnomer. This file sets up configs,
the accelerometer readings and the keypad.

Written primarily by Alex Richardson
*/


#include "lis3dsh.h"
//#include "accelerometer.h"


#define M_PI 3.14159265358979323846


//global constants
GPIO_InitTypeDef GPIO_Init;
TIM_HandleTypeDef TIM3_Handle;
LIS3DSH_InitTypeDef LIS3DSH_Initialize;
LIS3DSH_DRYInterruptConfigTypeDef LIS3DSH_Init_Config;
int global_counter;	
float acceleration_normalized[3];
float accel_temp[3];
int i;
float pitch;
float roll;

//FIR functions
#define Length 50
#define Order 5
float coeff[] = {.1,.15,.5,.15,.1};
float acceleration_reading_x[Length];		
float acceleration_reading_y[Length];		
float acceleration_reading_z[Length];

//initialize keypad state at 0, which would be idle
int keystate = 0;
char keypad[16] = {'1','2','3','A', '4','5','6','B','7','8','9','C','*','0','#','D'};
int input;
int input_flag;
int threshold_set_flag;

float calibration_param_matrix[4][3] = {
	{0.000975054748, -0.0000182180993, -0.0000000305652774},
	{-0.0000307680088, 0.00100024108, -0.00000438213129},
	{-0.00000617881222, -0.00000113744427, 0.000963388786},
	{0.00214820029, 0.00162097280, -0.0131357405}
};



//All the configs---------------------------------------------

void config_LEDs(void)
{	
	//Config LEDs
	GPIO_Init.Pin = GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15; 
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP; 
	GPIO_Init.Pull = GPIO_NOPULL;
	GPIO_Init.Speed = GPIO_SPEED_FAST; 

	//Init LED GPIO
	HAL_GPIO_Init(GPIOD, &GPIO_Init);
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
int FIR_C(float* InputArray, float* OutputArray, float* coeff, int length, int order)
{
	int n;
	int z;
	float weighted_total;

	//apply the filter
	for(n = order; n< length; n++){
		//calculate n
		weighted_total = 0;
		//implement the weighting factor by the subset of the data array dicated by the order number
		for( z=0; z< order; z++){
			weighted_total += coeff[z] * InputArray[n- order + z];
		}
		*(OutputArray++) = weighted_total;
	}
	
	return 0;
}

void get_acceleration(void)
{	
	int j;
	
	//Get the accelerometer readings
	LIS3DSH_ReadACC(accel_temp);
	
	*acceleration_reading_x =accel_temp[0];
	*(acceleration_reading_x) = *(acceleration_reading_x+1);

	
	float acceleration_filtered[3][Length - Order]; 
	//Filter Ax, Ay and Az values
	FIR_C(acceleration_reading_x, acceleration_filtered[0], coeff, Length, Order);
	FIR_C(acceleration_reading_y, acceleration_filtered[1], coeff, Length, Order);
	FIR_C(acceleration_reading_z, acceleration_filtered[2], coeff, Length, Order);
	
	
	//Calculate the normalized acceleration values for all 3 arrays
	for(i = 0; i < 3; i++) {
		for(j = 0; j < 3; j++) {
			acceleration_normalized[i] += acceleration_filtered[i][j]*calibration_param_matrix[j][i];
		}
		//adding 1*ACC10/20/30
		acceleration_normalized[i] += calibration_param_matrix[4][i];
	}
	
		
	//Print accelerometer readings
	for(i = 0; i < 3; i++)
	{
		printf("%f,", acceleration_normalized[i]);
	}
	
	printf("\n");
	
	//Calculate the roll and pitch angles
	pitch = atan2(acceleration_normalized[0],acceleration_normalized[2]) * 180/ M_PI;
	roll = atan2(acceleration_normalized[1],acceleration_normalized[2]) * 180/ M_PI;

	//reset normalized aceleration
	acceleration_normalized[0] =0 ;
	acceleration_normalized[1] =0 ;
	acceleration_normalized[2] =0 ;
}

/*Stuck keypad in here too. I would have named this supporting_functions.c
 if it wasn't already taken
 
 Description from document:Choose a port ofyour choice (let’s arbitrarily say PORTB)
and connect the pins to it in the order shown
	.PORTB0-3 to the rows(Configure initially as output)
	.PORTB4-7 to the columns (Configure initially as input, pull up mode enabled) 

*/

char get_key(void){

//note: logic is reversed, ie a 0 is a button push
	int column;
	int row;
	
//find column first
	//first 4 pins of the alphanumeric set to INPUT
	GPIO_Init.Pin = GPIO_PIN_3 | GPIO_PIN_1 | GPIO_PIN_5 | GPIO_PIN_6;
	GPIO_Init.Mode = GPIO_MODE_INPUT;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	GPIO_Init.Pull = GPIO_PULLUP; 
	HAL_GPIO_Init(GPIOD, &GPIO_Init);
	
	//last 4 pins of the alphanumeric set to OUTPUT
	GPIO_Init.Pin = GPIO_PIN_15 | GPIO_PIN_14 | GPIO_PIN_13 | GPIO_PIN_12;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	GPIO_Init.Pull = GPIO_NOPULL; 
	HAL_GPIO_Init(GPIOE, &GPIO_Init);
	
		if ( HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_3) == GPIO_PIN_RESET ) {
		column = 0;
	}
	else if ( HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_1) == GPIO_PIN_RESET ) {
		column = 1;
	}
	else if ( HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_5) == GPIO_PIN_RESET ) {
		column = 2;
	}
	else if ( HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_6) == GPIO_PIN_RESET ) {
		column =  3;
	}
	else {
		column = 4;
	}
	
//set row
	//first 4 pins set to out
	GPIO_Init.Pin = GPIO_PIN_3 | GPIO_PIN_1 | GPIO_PIN_5 | GPIO_PIN_6;
	GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	GPIO_Init.Pull = GPIO_NOPULL; 
	HAL_GPIO_Init(GPIOD, &GPIO_Init);
	
	//last 4 pins set to in
	GPIO_Init.Pin = GPIO_PIN_15 | GPIO_PIN_14 | GPIO_PIN_13 | GPIO_PIN_12;
	GPIO_Init.Mode = GPIO_MODE_INPUT;
	GPIO_Init.Speed = GPIO_SPEED_FAST;
	GPIO_Init.Pull = GPIO_PULLUP; 
	HAL_GPIO_Init(GPIOE, &GPIO_Init);
	
	
	if ( HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12) == GPIO_PIN_RESET ) {
		row = 0;
	}
	else if ( HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_13) == GPIO_PIN_RESET ) {
		row = 1;
	}
	else if ( HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14) == GPIO_PIN_RESET ) {
		row = 2;
	}
	else if ( HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_15) == GPIO_PIN_RESET ) {
		row = 3;
	}
	else {
		row = 4;
	}
	
	//idle
	if(keystate == 0){
		global_counter = 0;
		if(row <4 && column < 4)
			keystate++;
	}
	// debouncing, as suggested by the document
	else if(keystate == 1){
		global_counter = 0;
		//if its time to take a key value then shift previous number left by 1
		if(global_counter % 1000 ==0){
			if(isdigit(keypad[4*row*column]))
				input = input*10 + keypad[4*row+column];
			else{
				input_flag = 1;
			}
		}
		keystate++;
		
	}
	//key has been pressed
	else if(keystate == 2){
	 	if(row <4 && column < 4)
			keystate++;
	}
	//release button
	else if(keystate == 3){
		keystate = 0;
	}
	printf("%i", input);
	return input;
}


