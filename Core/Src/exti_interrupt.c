/*
********************************************************************************
* Filename      : exti_interrupt.c
* Version       : V1.00
* Created on    : 2023.10.19
* Programmer(s) : LLJ
********************************************************************************
*/

#include "exti_interrupt.h"

uint32_t last_time = 0;
uint8_t startflag = 0;
uint8_t interrupt_num = 0;
uint8_t	process_flag = 0;
uint8_t	receive_battery_state_data = 0x00;
uint8_t BatteryLevelBuf[RECVCOUNT] = {0};
uint8_t ShuntDownFlg = 2;
extern uint32_t receivetime;

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	uint32_t new_charge_stick = 0;
	static uint8_t ReceiveCount = 0;
	static uint32_t old_charge_stick = 0;

	if(GPIO_Pin == CHARGE_STATE_Pin){
		if(startflag == 0){
			if(1 == HAL_GPIO_ReadPin(CHARGE_STATE_GPIO_Port, CHARGE_STATE_Pin)){
				last_time = receivetime;
			}else{
				if((receivetime - last_time >=38)&&(receivetime - last_time <= 41)){
					startflag = 1;
					last_time = receivetime;
				}else{
					receivetime = 0;
					last_time = 0;
				}
			}
		}else{
			if(1 == HAL_GPIO_ReadPin(CHARGE_STATE_GPIO_Port, CHARGE_STATE_Pin)){
				interrupt_num++;
				receive_battery_state_data = receive_battery_state_data << 1;
				if((receivetime - last_time >= 3)&&(receivetime - last_time <= 4)){
					receive_battery_state_data = receive_battery_state_data | 0x01;
				}
			}else{
				last_time = receivetime;
			}
		}
		if(interrupt_num == 7){
			receive_battery_state_data = receive_battery_state_data << 1;
			BatteryLevelBuf[ReceiveCount] = receive_battery_state_data;
			//判断充电最低或者充电但电流为0时，为充电状态，即将进入关机；
			if(((receive_battery_state_data & 0x02) && ((receive_battery_state_data & 0x80) == 0)) ||
				(((receive_battery_state_data & 0x80) == 0) && ((receive_battery_state_data & 0x30) != 0)))
			{
				new_charge_stick = HAL_GetTick();
				if((new_charge_stick - old_charge_stick) > 500)
				{
					ShuntDownFlg = 1;
					old_charge_stick = new_charge_stick;
				}
			}else
			{
				if(ShuntDownFlg == 1)
				{
					BatteryLevelBuf[0] = receive_battery_state_data;
					BatteryLevelBuf[1] = receive_battery_state_data;
					BatteryLevelBuf[2] = receive_battery_state_data;
					BatteryLevelBuf[3] = receive_battery_state_data;
				}
				new_charge_stick = HAL_GetTick();
				old_charge_stick = new_charge_stick;
			}
			ReceiveCount++;
			if(ReceiveCount > (RECVCOUNT-1))
			{
				ReceiveCount = 0;
			}
			interrupt_num = 0;
			last_time = 0;
			receivetime = 0;
			startflag = 0;
			process_flag = 1;
		}
	}
}

