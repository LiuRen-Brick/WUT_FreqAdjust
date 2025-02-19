/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f1xx_it.h"
#include "tim.h"
#include "stm32f1xx_hal_tim.h"
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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define WAVE_A_EN_Pin GPIO_PIN_0
#define WAVE_A_EN_GPIO_Port GPIOC
#define MOTOR_GATE_Pin GPIO_PIN_3
#define MOTOR_GATE_GPIO_Port GPIOA
#define PWM_MOTOR_B_Pin GPIO_PIN_6
#define PWM_MOTOR_B_GPIO_Port GPIOA
#define PWM_MOTOR_A_Pin GPIO_PIN_7
#define PWM_MOTOR_A_GPIO_Port GPIOA
#define VBAT_ADC_Pin GPIO_PIN_1
#define VBAT_ADC_GPIO_Port GPIOB
#define BOOT1_Pin GPIO_PIN_2
#define BOOT1_GPIO_Port GPIOB
#define USART3_TX_Pin GPIO_PIN_10
#define USART3_TX_GPIO_Port GPIOB
#define USART3_RX_Pin GPIO_PIN_11
#define USART3_RX_GPIO_Port GPIOB
#define KEY_CONTROL_Pin GPIO_PIN_6
#define KEY_CONTROL_GPIO_Port GPIOC
#define CHARGE_STATE_Pin GPIO_PIN_9
#define CHARGE_STATE_GPIO_Port GPIOC
#define CHARGE_STATE_EXTI_IRQn EXTI9_5_IRQn
#define CONTROL_CLOSE_Pin GPIO_PIN_8
#define CONTROL_CLOSE_GPIO_Port GPIOA
#define USART1_TX_Pin GPIO_PIN_9
#define USART1_TX_GPIO_Port GPIOA
#define USART1_RX_Pin GPIO_PIN_10
#define USART1_RX_GPIO_Port GPIOA
#define MPDCDC_EN_Pin GPIO_PIN_12
#define MPDCDC_EN_GPIO_Port GPIOA
#define WAVE_B_EN_Pin GPIO_PIN_15
#define WAVE_B_EN_GPIO_Port GPIOA
#define B_SCLK_Pin GPIO_PIN_10
#define B_SCLK_GPIO_Port GPIOC
#define B_SDATA_Pin GPIO_PIN_11
#define B_SDATA_GPIO_Port GPIOC
#define B_FSYNC_Pin GPIO_PIN_12
#define B_FSYNC_GPIO_Port GPIOC
#define A_SCLK_Pin GPIO_PIN_2
#define A_SCLK_GPIO_Port GPIOD
#define A_SDATA_Pin GPIO_PIN_3
#define A_SDATA_GPIO_Port GPIOB
#define A_FSYNC_Pin GPIO_PIN_4
#define A_FSYNC_GPIO_Port GPIOB
#define LTDCDC_EN_Pin GPIO_PIN_7
#define LTDCDC_EN_GPIO_Port GPIOB
#define WAVE_BCH_EN_Pin GPIO_PIN_8
#define WAVE_BCH_EN_GPIO_Port GPIOB
#define WAVE_ACH_EN_Pin GPIO_PIN_9
#define WAVE_ACH_EN_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
