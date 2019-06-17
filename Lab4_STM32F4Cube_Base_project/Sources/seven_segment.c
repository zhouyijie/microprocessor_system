
#include "main.h"

void display_num(int num){
	//clear before displaying
	HAL_GPIO_WritePin (GPIOD , GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 |GPIO_PIN_4 |GPIO_PIN_5 |GPIO_PIN_6 |GPIO_PIN_7 , GPIO_PIN_RESET);
		//printf("input number is %i \n" , num);
		switch (num) 
	{
		//0. Need pins: 0,1,2,3,4,5
		case 0:
			HAL_GPIO_WritePin (GPIOD, GPIO_PIN_0 |GPIO_PIN_1 |GPIO_PIN_2 |GPIO_PIN_3 |GPIO_PIN_4 | GPIO_PIN_5, GPIO_PIN_SET);
			//printf("display 0 \n");
			break;
		//1. Need pins: 1,2
		case 1:
			HAL_GPIO_WritePin (GPIOD , GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_SET); 
			//printf("display 1 \n");
			break;
		//2. Need pins: 0,1,3,4,6
		case 2:
			HAL_GPIO_WritePin (GPIOD , GPIO_PIN_0 | GPIO_PIN_1 |GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_6 , GPIO_PIN_SET); 
			//printf("display 2 \n");
			break;
		//3. Need pins: 0,1,2,3,6
		case 3:
			HAL_GPIO_WritePin (GPIOD , GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_6, GPIO_PIN_SET);
			//printf("display 3 \n");
			break;
		//4. Need pins: 1,2,5,6
		case 4:
			HAL_GPIO_WritePin (GPIOD , GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_5 | GPIO_PIN_6, GPIO_PIN_SET); 	
			//printf("display 4 \n");
			break;
		//5. Need pins: 0,2,3,5,6
		case 5:
			HAL_GPIO_WritePin (GPIOD ,GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3| GPIO_PIN_5| GPIO_PIN_6, GPIO_PIN_SET); 	
			//printf("display 5 \n");
			break;
		//6. Need pins 0,2,3,4,5,6
		case 6:
			HAL_GPIO_WritePin (GPIOD , GPIO_PIN_0 | GPIO_PIN_2| GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6, GPIO_PIN_SET); 
			//printf("display 6 \n");
			break;
		//7. Need pins 0,1,2
		case 7:
			HAL_GPIO_WritePin (GPIOD ,GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_SET); 			
			//printf("display 7 \n");
			break;
		//8. Need all pins
		case 8:
			HAL_GPIO_WritePin (GPIOD , GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2| GPIO_PIN_3 |GPIO_PIN_4 |GPIO_PIN_5 |GPIO_PIN_6, GPIO_PIN_SET); 		
			//printf("display 8 \n");
			break;
		//9. Need pins 0,1,2,5,6
		case 9:
			HAL_GPIO_WritePin (GPIOD , GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 |GPIO_PIN_5 |GPIO_PIN_6, GPIO_PIN_SET); 	
			//printf("display 9 \n");
			break;
	}
}
void update_7seg_disp(float f){
	int number = f*10;
	int position = 0;
	if(number < 1000){
		while (position != 3){
			HAL_GPIO_WritePin(GPIOB , GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4, GPIO_PIN_RESET);
			//Set the value, then set the correct digit to turn on
			switch (position){
				case 0:
					//first digit.
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET) ;
					display_num(number % 10);
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_2, GPIO_PIN_SET);
					//printf("first digit, input number is %i \n" , number);
					
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
					
				break;
				
				case 1:
					//second digit and decimal point
					//decimal point
					//printf("second digit, input number is %i \n" , number);
					
					
					HAL_GPIO_WritePin (GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
					display_num(number % 10);
					HAL_GPIO_WritePin (GPIOD, GPIO_PIN_7, GPIO_PIN_SET) ;
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET) ;				
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);
				break;
				
				case 2:
					//third digit
					//printf("third digit, input number is %i \n" , number);
					
					
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);	
					display_num(number % 10);	
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET) ;
					HAL_GPIO_WritePin (GPIOB, GPIO_PIN_2, GPIO_PIN_RESET);
				break;
				
			}
			//printf("position is %i \n" , position);
			position ++;
			number /= 10;
		}
	}
}
