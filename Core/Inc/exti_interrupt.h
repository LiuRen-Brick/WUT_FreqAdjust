/*
********************************************************************************
* Filename      : exti_interrupt.h
* Version       : V1.00
* Created on    : 2023.10.19
* Programmer(s) : LLJ
********************************************************************************
*/
#ifndef INC_EXTI_INTERRUPT_H_
#define INC_EXTI_INTERRUPT_H_
#include "main.h"
#include "tim.h"

#define RECVCOUNT  4

extern uint8_t  process_flag;
extern uint8_t  receive_battery_state_data;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);

#endif /* INC_EXTI_INTERRUPT_H_ */
