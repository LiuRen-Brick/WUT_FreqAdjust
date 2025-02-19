/*
********************************************************************************
* Filename      : battery.c
* Version       : V1.00
* Created on    : 2023.10.19
* Programmer(s) : LLJ
********************************************************************************
*/
#include "battery.h"
#include "exti_interrupt.h"
#include "usart.h"
#include "control.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

enum battery_level	battery_level_state = boost_battery_level1;
extern enum Ultransound_Work_State  Ultrasound_state;
extern uint8_t BatteryLevelBuf[RECVCOUNT];
extern uint8_t FinRes;
extern uint8_t Low_Battery_Flg;
extern uint8_t CompleteFlg;
uint8_t send_battery_state_data = boost_battery_level5;				//初始为放电状态，避免出现开机充电界面
uint8_t send_battery_state_data_old = boost_battery_level5;
uint8_t battery_voltage_alarm_flag = 0;
uint8_t filter_num_count;
uint8_t battery_txbuffer[14]={0xEE,0xB5,0x10,0x00,0x00,0x00,0x00,0x10,0x01,0x01,0xFF,0xFC,0xFF,0xFF};

/*
 * @brief	Battery_LevelReceive
 * @param   none
 * @retval  none
 * @func	将接收到的电量转为等级
 */
void Battery_LevelReceive(void)
{
	static uint8_t	battery_voltage_data = 0xff;

	//判断4次接收到的电量值是否相等，避免采样误差
	if((BatteryLevelBuf[0] == BatteryLevelBuf[1]) &&
	   (BatteryLevelBuf[1] == BatteryLevelBuf[2]) &&
	   (BatteryLevelBuf[2] == BatteryLevelBuf[3]))
	{
		if((BatteryLevelBuf[0] == 0x80) || (BatteryLevelBuf[0] == 0x84))   //判断充放电状态是顺便判断电流，防止误判
		{
			battery_voltage_data = 0xAA;
		}else
		{
			if(((BatteryLevelBuf[0] & 0x02) == 0) && ((BatteryLevelBuf[0] & 0x80) == 0x80))    //第二位为0的时候为放电
			{

				battery_voltage_data = BatteryLevelBuf[0] & 0xf0;							   //放电
				if((BatteryLevelBuf[0] < 0x98) && (Ultrasound_state != WORK_STATE))
				{
					Low_Battery_Flg = 1;
				}
			}else if(BatteryLevelBuf[0] != 0xFF)
			{
				battery_voltage_data = BatteryLevelBuf[0] & 0x70;
			}else
			{

			}
		}


		switch(battery_voltage_data)
		{
			//0x00 - 0x70电池处于充电状态
			case	0x00:
					battery_level_state =  charge_battery_level1;
						break;

			case	0x10:
					battery_level_state =  charge_battery_level2;
						break;

			case	0x20:
					battery_level_state =  charge_battery_level3;
						break;

			case	0x30:
					battery_level_state =  charge_battery_level4;
						break;

			case	0x70:
					battery_level_state =  charge_battery_level5;
						break;

			//Boost 放电状态
			case	0xAA:
					battery_level_state =  boost_battery_level1;
						break;

			case	0x80:
					battery_level_state =  boost_battery_level2;
						break;

			case	0x90:
					battery_level_state =  boost_battery_level3;
						break;

			case	0xA0:
					battery_level_state =  boost_battery_level4;
						break;
			case	0xB0:

					battery_level_state =  boost_battery_level5;

						break;
			default :
						break;

		}
		send_battery_state_data = battery_level_state;
	}
}

/*
 * @brief	Battery_LevelSend
 * @param   none
 * @retval  none
 * @func	向屏幕发送电池电量信息
 */
void Battery_LevelSend(void)
{
	static uint8_t chact = boost_battery_level1;
	static uint8_t ChCount = 0;
	static uint8_t DisChCount = 0;

	//充电显示400ms为周期发送
	//放电显示200ms为周期发送
	if(send_battery_state_data < boost_battery_level1)
	{
		if(ChCount > 19)
		{
			ChCount = 0;
		}
		if(ChCount == 0)
		{
			//充电循环动画，
			if((chact < boost_battery_level1) && (chact >= send_battery_state_data))
			{
#ifdef CHARGE_WORK
				battery_txbuffer[2] = 0X12;
#else
				battery_txbuffer[2] = 0X11;
#endif
				battery_txbuffer[6] = chact;
				DMA_USART3_Tx_Data(battery_txbuffer, 14);
				chact++;
				FinRes = Bat_Charge;
			}else
			{
				chact = send_battery_state_data;
				send_battery_state_data_old = send_battery_state_data;
			}
		}
		DisChCount = 0;
		ChCount++;
	}else
	{
		if(DisChCount > 9)
		{
			DisChCount = 0;
		}

		if(DisChCount == 0)
		{
			battery_txbuffer[2] = 0X12;
			if(CompleteFlg == 1)
			{
				if((send_battery_state_data > send_battery_state_data_old) && (send_battery_state_data_old > charge_battery_level5))
				{
					battery_txbuffer[6] = send_battery_state_data_old - 5;
				}else
				{
					send_battery_state_data_old = send_battery_state_data;
					battery_txbuffer[6] = send_battery_state_data_old - 5;
				}
			}else
			{
				battery_txbuffer[6] = send_battery_state_data - 5;
			}
			//低电量且不在工作状态时发送低电量预警
			if((Low_Battery_Flg == 1) && (Ultrasound_state != WORK_STATE))
			{
				battery_txbuffer[6] = 6;
			}
			DMA_USART3_Tx_Data(battery_txbuffer, 14);
		}
		DisChCount++;
		ChCount = 1;
	}
}
