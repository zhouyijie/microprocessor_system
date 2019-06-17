/**
  ******************************************************************************
  * @file    HAL/HAL_TimeBase/Inc/main.h 
  * @author  MCD Application Team
  * @version   V1.2.4
  * @date      2017/03/17
  * @brief   Header for config_files.c module
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
	
#ifndef __CONFIG_FILES_H
#define __CONFIG_FILES_H	

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_adc.h"
#include "stm32f4xx_hal_tim.h"
#include "supporting_functions.h"
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "RTE_Components.h"             // Component selection
#include "math.h"
#include "lis3dsh.h"


//For ADC
extern ADC_InitTypeDef ADC_init;
extern ADC_ChannelConfTypeDef ADC_ChannelStruct;
extern ADC_HandleTypeDef ADC1_handler;

//For accelerometer
extern LIS3DSH_InitTypeDef LIS3DSH_Initialize;
extern LIS3DSH_DRYInterruptConfigTypeDef LIS3DSH_Init_Config;

/* Functions ---------------------------------------------------------*/
extern void config_accelerometer(void);
extern void config_adc(void);
extern void config_interrupt(void);
extern void config_timer(void);
extern void config_7seg (void);


#endif /* __CONFIG_FILES_H */	
	/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/