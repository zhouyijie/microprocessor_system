
/* Includes ------------------------------------------------------------------*/
#include "main.h"


/* Private variables ---------------------------------------------------------*/

float acceleration_normalized[3];
float accel_temp[3];
int i;
float pitch;
float roll;
float duty_cycle;

float display_temp, display_pitch, display_roll;
int display_angle_counter; 

float acceleration_reading_x[Length]; 
float acceleration_reading_y[Length]; 
float acceleration_reading_z[Length];

//Lab4 additional variables
int display_mode;
int ext_io_flag = 0;
float threshold;
int input_value;
//int input_value;

#define TOLERANCE 4.0

//functions used in extern
void Thread_angle (void const *argument);  
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

osThreadId tid_angle;// thread id
osThreadDef(Thread_angle, osPriorityNormal, 1, 0);

int start_Thread_angle (void) {
  
   printf("start thread angle\n");
  tid_angle = osThreadCreate(osThread(Thread_angle), NULL); // Start calculation_Thread
  if (!tid_angle)
      return(-1); 
  return(0);
} 

void get_acceleration(int length, int order)
{ 
	 int j;
     int i;
	 float acceleration_filtered_x[Length - Order];
     float acceleration_filtered_y[Length - Order];
     float acceleration_filtered_z[Length - Order];
		 float pitch_temp;
		 float roll_temp;
     //Get the accelerometer readings
     LIS3DSH_ReadACC(accel_temp);
     //shift Array
     for ( i = 0; i < length ;i++){
     acceleration_reading_x[length-i-1] = acceleration_reading_x[length-i-2];
     acceleration_reading_y[length-i-1] = acceleration_reading_y[length-i-2];
     acceleration_reading_z[length-i-1] = acceleration_reading_z[length-i-2];
     }
     acceleration_reading_x[0]=accel_temp[0];
     acceleration_reading_y[0]=accel_temp[1];
     acceleration_reading_z[0]=accel_temp[2];
     //printf("acceleration values are: %f, %f, %f. \n", acceleration_reading_x[0],acceleration_reading_y[0],acceleration_reading_z[0]);

     //Filter Ax, Ay and Az values
     FIR_C(acceleration_reading_x, acceleration_filtered_x, coeff, length, order);
     FIR_C(acceleration_reading_y, acceleration_filtered_y, coeff, length, order);
     FIR_C(acceleration_reading_z, acceleration_filtered_z, coeff, length, order);
     // printf("acceleration values are:");
     // for(i=0;i<3;i++){
     // printf("%f ,",acceleration_filtered_x[i]);
     // printf("%f ,",acceleration_filtered_y[i]);
     // printf("%f ,",acceleration_filtered_z[i]);
     // printf("\n");
     // }

     //Calculate the normalized acceleration values for all 3 arrays. Ultimately in our program this is just a mean function

     for( j = 0; j < length; j++) {
     acceleration_normalized[0] += acceleration_filtered_x[j];
     }
     acceleration_normalized[0] = acceleration_normalized[0]/length; 
     for( j = 0; j < length; j++) {
     acceleration_normalized[1] += acceleration_filtered_y[j];
     }
     acceleration_normalized[1] = acceleration_normalized[1]/length; 
     for( j = 0; j < length; j++) {
     acceleration_normalized[2] += acceleration_filtered_z[j];
     }
     acceleration_normalized[2] = acceleration_normalized[2]/length; 

     //Calculate the roll and pitch angles
     pitch_temp = acceleration_normalized[0]/acceleration_normalized[2];
     roll_temp = acceleration_normalized[1]/acceleration_normalized[2];
     pitch = atan(pitch_temp)*180/M_PI+90;
     roll = atan(roll_temp)*180/M_PI+45;

     //printf("pitch is: %f, roll is: %f \n",pitch,roll);
     //reset normalized aceleration
     acceleration_normalized[0] =0 ;
     acceleration_normalized[1] =0 ;
     acceleration_normalized[2] =0 ;
}


void Thread_angle (void const *argument) {
	display_pitch = 0.0;
	display_roll = 0.0;
	display_angle_counter = 0;

	while(1){
		osSignalWait(2, osWaitForever);
		osMutexWait(angle_mutex, osWaitForever);
		printf("Thread_angle while loop started\n");
		//printf("it thinks display mode is %i", display_mode);
		//input_value = get_key();

		//Check if it is in angle display mode
		if (display_mode == 1) {		
			
			//If the EXTI0 callback function is called and flag is set to active, read accelerometer values
			if(ext_io_flag == 1) {
				
				//Get the accelerometer readings
				get_acceleration(Length, Order);
				
				//Store pitch angle for display
				if ( p_or_r == 0) {
					display_pitch = absolute(input_value - pitch);
				}
					
				//Store roll angle for display
				if (p_or_r == 1) {
					display_roll = absolute(input_value - roll);
				}		
				
				//Slow down the updating of the angle display to 5Hz (25Hz of acc readings/5)
				if(display_angle_counter % 5 == 0) {
					display = p_or_r == 0 ?  display_pitch: display_roll;
					display_angle_counter = 0;
				}
			
				//Reset the flag
				ext_io_flag = 0;
			}//ends extio interrupt
		}//ends display==1
		osMutexRelease(angle_mutex);
		osSignalClear(tid_angle, 2);
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	//Update this counter only when you are in the angle display mode
	if(display_mode == 1)
		display_angle_counter++;
	//Set EXTI0 interrupt flag to true
	ext_io_flag = 1;
	osSignalSet(tid_angle, 2);
	//printf("\n ext last word?\n");

}
