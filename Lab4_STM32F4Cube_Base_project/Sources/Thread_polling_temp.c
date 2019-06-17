/* Includes ------------------------------------------------------------------*/

#include "main.h"

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
extern float coeff[Order];
int temp_count;
int alarm_on;
int pina;
int shown_temp;
/* USER CODE END PFP */
#define overheat_c	60.0
#define overheat_f 60.0
/* USER CODE BEGIN 0 */


void Thread_polling_temp (void const *argument);

//create RTOS thread for temperature

osThreadId tid_Thread_polling_temp;                              // thread id
osThreadDef(Thread_polling_temp, osPriorityNormal, 1, 0);	

int start_Thread_polling_temp (void) {

     
     printf("start thread polling temp\n");
  tid_Thread_polling_temp = osThreadCreate(osThread(Thread_polling_temp ), NULL); // Start Temp polling Thread
  if (!tid_Thread_polling_temp)
      return(-1); 
  return(0);
}

float mean(float* input, int length){
    
	int i;
	float sum = 0;
	for(i = 0; i < length; i++){
		sum += input[i];
	}
	return (sum / length);
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

//this thread contantly polls the temperature
void Thread_polling_temp (void const *argument) {
	float temp[Length];
	float temp_filtered[Length-Order + 1];
    //set initial temp as room temp
	shown_temp = 27;
	temp_count = 0;     

	alarm_on = 0;
    //default to C
	pina = 0;

	while(1){
        printf("Thread_polling_temp while loop started\n");
        
        //10 ms delay
        osDelay(10);
         printf("delay finished"); 
        //if (HAL_ADC_Start(&ADC1_handler) == HAL_OK) {
            // printf("start sensor_data\n");
            //initialize F or C for this cycle of readings. DEPRECATED FOR LAB 4
            //if(temp_count == 0)
                //pina = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0);
            temp[temp_count % 50] = sensor_data(pina);
            //get readings for the temp aray to be filtered

            //printf("read pina value is %i", pina);

            FIR_C(temp, temp_filtered, coeff, Length, Order);
            shown_temp = mean(temp_filtered, (Length-Order+1));
            printf("temperature value is %i\n", shown_temp);

            //Start alarm if overheating and change alarm status. pina=1 is F, 0 is C
            if(pina && (shown_temp > overheat_f)){
                    alarm_on = 1;
                
            }
            else if(!pina &&(shown_temp > overheat_c) ){
                    alarm_on = 1;
            }
            else{
                alarm_on =0;
            }
            
            //Clear the EOC flag
			//__HAL_ADC_CLEAR_FLAG(&ADC1_handler,ADC_FLAG_EOC);
        //}//ends ADC validity check
	}//end while
}//end main
