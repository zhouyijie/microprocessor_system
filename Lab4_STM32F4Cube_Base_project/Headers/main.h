/**
  ******************************************************************************
  * @file    HAL/HAL_TimeBase/Inc/main.h 
  * @author  MCD Application Team
  * @version   V1.2.4
  * @date      2017/03/17
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_tim.h"
#include "supporting_functions.h"
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "RTE_Components.h"             // Component selection
#include "math.h"
#include "lis3dsh.h"
#include "config_files.h"
#include "various_functions.h"


/* Definitions -----------------------------------------------------------------*/

//FIR function global definitions
#define Length 50
//Order MUST be odd for my coeff generator to work
#define Order 25

#define M_PI 3.14159265358979323846


/* Exported types ------------------------------------------------------------*/
extern osMutexId angle_mutex, temp_mutex;
extern int ext_io_flag, display_mode, temp_flag, input_flag, input_value, alarm_on, p_or_r;
extern float display;
extern float display_temp, display_pitch, display_roll;
extern float coeff[Order];
extern GPIO_InitTypeDef GPIO_Init;
extern TIM_HandleTypeDef TIM3_handler;

extern int start_Thread_temp(void);
extern int start_Thread_angle(void);
extern int start_Thread_polling_temp(void);
extern void Thread_angle (void const *argument);
extern void Thread_temp(void const *argument);
extern void Thread_polling_temp(void const *argument);

/* Exported functions ------------------------------------------------------- */

extern int get_key(void);
extern void determine_input (int key);
extern void update_7seg_disp(float f);
extern void display_num(int num);

#endif /* __MAIN_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/