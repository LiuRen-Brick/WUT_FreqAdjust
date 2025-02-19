/*
********************************************************************************
* Filename      : control.c
* Version       : V1.00
* Created on    : 2023.10.26
* Programmer(s) : LLJ
********************************************************************************
*/
#ifndef INC_CONTROL_H_
#define INC_CONTROL_H_

#include "main.h"
#include "tim.h"
#include "ad9833.h"
#include "gpio.h"
#include "mcu_gpio.h"
#include "battery.h"
#include "exti_interrupt.h"
#include "serial_eeprom.h"
#include "emulation_ee.h"
#include "usart.h"
#include "adc.h"
#include "stm32f1xx_it.h"

#define CHARGE_WORK

enum Ultransound_Work_State
{
	NULL_STATE = 0,
	CHARGE_STATE = 1,
	STANDBY_STATE = 2,
	PUASE_STATE = 3,
	WORK_STATE = 4,
	CLOSE_STATE = 5
};

typedef enum
{
	Vol_Low = 1,
	Bat_Charge,
}STOP_REASON;

typedef struct
{
	uint8_t Year;
	uint8_t Mon;
	uint8_t Day;
	uint8_t Hour;
	uint8_t Min;
}STARY_TIME;

void DectoHex( uint32_t dec, uint8_t *hex, uint16_t length );
uint8_t HextoDec( uint8_t* buffer, uint16_t length );

void System_Init(void);
void Usart3_ReceiveData(void);
void Ultra_FourStates(void);
void Log_Writer2Flash(void);
void Warning_LowBattery(void);

#endif /* INC_CONTROL_H_ */
