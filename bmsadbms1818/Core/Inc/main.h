/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define FET_CUR_A2_Pin GPIO_PIN_3
#define FET_CUR_A2_GPIO_Port GPIOC
#define OPAMP_INP_Pin GPIO_PIN_0
#define OPAMP_INP_GPIO_Port GPIOA
#define OPAMP_INM_Pin GPIO_PIN_1
#define OPAMP_INM_GPIO_Port GPIOA
#define OPAMP_OUT_Pin GPIO_PIN_3
#define OPAMP_OUT_GPIO_Port GPIOA
#define dc_dc_15V_Pin GPIO_PIN_4
#define dc_dc_15V_GPIO_Port GPIOA
#define FAN_PWM_Pin GPIO_PIN_5
#define FAN_PWM_GPIO_Port GPIOA
#define HV_div1_Pin GPIO_PIN_7
#define HV_div1_GPIO_Port GPIOA
#define Therm_spare1_Pin GPIO_PIN_4
#define Therm_spare1_GPIO_Port GPIOC
#define Therm_spare2_Pin GPIO_PIN_5
#define Therm_spare2_GPIO_Port GPIOC
#define LED_RED_Pin GPIO_PIN_0
#define LED_RED_GPIO_Port GPIOB
#define LED_GRN_Pin GPIO_PIN_1
#define LED_GRN_GPIO_Port GPIOB
#define HV_div2_Pin GPIO_PIN_2
#define HV_div2_GPIO_Port GPIOB
#define FAN_TACH_Pin GPIO_PIN_10
#define FAN_TACH_GPIO_Port GPIOB
#define PADxx_Pin GPIO_PIN_12
#define PADxx_GPIO_Port GPIOB
#define PAD7_Pin GPIO_PIN_15
#define PAD7_GPIO_Port GPIOB
#define DUMP2_Pin GPIO_PIN_6
#define DUMP2_GPIO_Port GPIOC
#define DUMP_NOT_Pin GPIO_PIN_8
#define DUMP_NOT_GPIO_Port GPIOC
#define CSB_Pin GPIO_PIN_15
#define CSB_GPIO_Port GPIOA
#define DUMP_Pin GPIO_PIN_10
#define DUMP_GPIO_Port GPIOC
#define HEATER_NOT_Pin GPIO_PIN_11
#define HEATER_NOT_GPIO_Port GPIOC
#define HEATER_Pin GPIO_PIN_12
#define HEATER_GPIO_Port GPIOC
#define BQ_LD_Pin GPIO_PIN_2
#define BQ_LD_GPIO_Port GPIOD
#define WDT_Pin GPIO_PIN_7
#define WDT_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
// USART/UART assignments
#define HUARTMON  huart1 // uart  for PC monitoring

#define DEFAULTTASKBIT00 (1 << 0)  // Task notification bit (from ADCtask.c)
#define DEFAULTTASKBIT01 (1 << 1)  // Task notification bit (from BQtask.c)
#define DEFAULTTASKBIT02 (1 << 2)  // Task notification bit (from Mailbox)

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
