/*
********************************************************************************
* Filename      : mcu_gpio.c
* Version       : V1.00
* Created on    : 2023.09.13
* Programmer(s) : LLJ
********************************************************************************
*/
#include "mcu_gpio.h"

void Mcu_GpioInit(void)
{
	HAL_GPIO_WritePin(CONTROL_CLOSE_GPIO_Port, CONTROL_CLOSE_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(MPDCDC_EN_GPIO_Port, MPDCDC_EN_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LTDCDC_EN_GPIO_Port, LTDCDC_EN_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(MOTOR_GATE_GPIO_Port, MOTOR_GATE_Pin, GPIO_PIN_RESET);

	HAL_GPIO_WritePin(WAVE_A_EN_GPIO_Port, WAVE_A_EN_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(WAVE_B_EN_GPIO_Port, WAVE_B_EN_Pin, GPIO_PIN_RESET);

	HAL_GPIO_WritePin(KEY_CONTROL_GPIO_Port, KEY_CONTROL_Pin, GPIO_PIN_RESET);

	__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, 0);	//PWM_MOTOR_B
	__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, 0);	//PWM_MOTOR_A

}
