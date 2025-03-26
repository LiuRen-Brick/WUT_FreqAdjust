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

uint8_t BatStaCode_Boost[16] =
{
	0x80, 0x84, 0x88, 0x8C, 0x90, 0x94, 0x98, 0x9C, 0xA0, 0xA4, 0xA8, 0xAC, 0xB0, 0xB4, 0xB8, 0xBC,
};

uint8_t BatStaLevel_BoostI[16] =
{
	boost_battery_level1,
	boost_battery_level1,
	boost_battery_level2,
	boost_battery_level2,
	boost_battery_level2,
	boost_battery_level3,
	boost_battery_level3,
	boost_battery_level3,
	boost_battery_level3,
	boost_battery_level4,
	boost_battery_level4,
	boost_battery_level4,
	boost_battery_level4,
	boost_battery_level5,
	boost_battery_level5,
	boost_battery_level5,
};

uint8_t BatStaLevel_BoostW[16] =
{
	boost_battery_level1,
	boost_battery_level2,
	boost_battery_level2,
	boost_battery_level2,
	boost_battery_level3,
	boost_battery_level3,
	boost_battery_level3,
	boost_battery_level3,
	boost_battery_level4,
	boost_battery_level4,
	boost_battery_level4,
	boost_battery_level4,
	boost_battery_level5,
	boost_battery_level5,
	boost_battery_level5,
	boost_battery_level5,
};

uint8_t BatStaCode_Charge[17] =
{
	0x02, 0x06, 0x0A, 0x0E,	0x12, 0x16, 0x1A, 0x1E, 0x22, 0x26, 0x2A, 0x2E, 0x32, 0x36, 0x38, 0x3A, 0x7C,
};

uint8_t BatStaLevel_Charge[17] =
{
	charge_battery_level1,
	charge_battery_level1,
	charge_battery_level1,
	charge_battery_level1,
	charge_battery_level2,
	charge_battery_level2,
	charge_battery_level2,
	charge_battery_level2,
	charge_battery_level3,
	charge_battery_level3,
	charge_battery_level3,
	charge_battery_level3,
	charge_battery_level4,
	charge_battery_level4,
	charge_battery_level4,
	charge_battery_level4,
	charge_battery_level5,
};

static uint8_t BatteryLevelGat(const uint8_t stacode,const uint8_t devsta);
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

		battery_level_state = BatteryLevelGat(battery_voltage_data,Ultrasound_state);

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

static uint8_t BatteryLevelGat(const uint8_t stacode,const uint8_t devsta)
{
	uint8_t batlevel = boost_battery_level5;
	uint8_t status = STANDBY_STATE;
	uint8_t tag_i = 0;

	status = devsta;

	switch(status)
	{
	case CHARGE_STATE:

		for(tag_i = 0;tag_i < 17;tag_i++)
		{
			if(stacode == BatStaCode_Charge[tag_i])
			{
				batlevel = BatStaLevel_Charge[tag_i];
				break;
			}
		}

		break;

	case WORK_STATE:

		for(tag_i = 0;tag_i < 17;tag_i++)
		{
			if(stacode == BatStaCode_Boost[tag_i])
			{
				batlevel = BatStaLevel_BoostW[tag_i];
				break;
			}
		}

		break;

	default:

		for(tag_i = 0;tag_i < 17;tag_i++)
		{
			if(stacode == BatStaCode_Boost[tag_i])
			{
				batlevel = BatStaLevel_BoostI[tag_i];
				break;
			}
		}

		break;
	}


	return batlevel;
}
