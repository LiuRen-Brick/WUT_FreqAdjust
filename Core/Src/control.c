/*
********************************************************************************
* Filename      : control.c
* Version       : V1.00
* Created on    : 2023.10.19
* Programmer(s) : LLJ
********************************************************************************
*/

#include "control.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "usart.h"

#define SIGNLE
#define PULSE_PERIOD 100
#define ULTRA_BASE	551
#define MOTOR_BASE  216

extern uint32_t Motor_Worktime;
extern uint8_t CompleteFlg;
extern enum battery_level	battery_level_state;
enum Ultransound_Work_State  Ultrasound_state = NULL_STATE ;
extern uint32_t ultrasound_standby_time,tim4_count1,tim4_count2;
uint8_t  Comd_funparameter_txbuffer[14]={0xEE,0xB5,0x13,0xBC,0x00,0x00,0x01,0x11,0x30,0x30,0xFF,0xFC,0xFF,0xFF};
#ifdef SIGNLE
	uint8_t duty_cycle_count = 100;
	uint16_t duty_tim = 100;
	uint32_t duty_count = 50000;
#else
	uint32_t duty_cycle_count = 50;
#endif
uint16_t Change_Paramete_Flg[3];
uint16_t Ultrasound_Parem_Read_Buff[3];
uint16_t Ultrasound_Parem_Write_Buff[3];
uint8_t Frequency_Parameter_A;
uint8_t Frequency_Parameter_B;
uint8_t Vibration_Paramete;

uint8_t Comd_funparameter_Frequency_txbuffer_A;
uint8_t Comd_funparameter_Frequency_txbuffer_B;
uint8_t Comd_funparameter_Vibration_txbuffer;

uint32_t u32_frequency_parameter;
uint32_t u32_vibration_parameter;

/***********************************日志保存参数***********************************************/
uint8_t FinRes = 0;
uint8_t WorkParm[8];
uint8_t WorkStartFlg = 0;
uint8_t WorkFinishFlg = 0;
uint16_t WorkCount = 0;
uint32_t WorkTime = 0;
uint8_t Cmd_GetTim[6] = {0xEE,0x82,0xFF,0xFC,0xFF,0xFF};
STARY_TIME Start_Time;
/**********************************低电量预警参数***********************************************/
uint8_t Low_Battery_Flg = 0;
uint32_t Charge_Time = 0;
/**********************************参数锁定**************************************************/
uint8_t LockOn_ParamFlg = 0;
uint32_t ShuntDowmCount = 0;
uint16_t PowerFlg = 0;

uint16_t FreqAdjustParam[2] = {0};

/*****************************************************************************************
 * FLASH_SAVE_ADDR		            参数修改标志位
 * FLASH_SAVE_ADDR+0x00000400		低电量
 * FLASH_SAVE_ADDR+0x00000800       参数锁定
 * FLASH_SAVE_ADDR+0x00000C00       超声参数
 * FLASH_SAVE_ADDR+0x00001000       充电后自动关机
 * FLASH_SAVE_ADDR+0x00001400       预留
 * FLASH_SAVE_ADDR+0x00001800       程序加密
 * FLASH_SAVE_ADDR+0x00001C00       工作次数
 */
static void SetPWMDutyCycleAndFrequency(uint32_t pulsetime);

void DectoHex( uint32_t dec, uint8_t *buffer, uint16_t length )
{
	uint8_t i;
    for( i = length - 1; i >= 0; i-- )
    {
    	buffer[i] = ( dec % 256 ) & 0xFF;
        dec /= 256;
    }
}

uint8_t HextoDec( uint8_t* buffer, uint16_t length )
{
	uint8_t i;
    uint8_t rslt = 0;
    for( i = 0; i < length; i++ )
    {
        rslt += ( uint32_t )( buffer[i] ) << ( 8 * ( length - 1 - i ) );
    }
    return rslt;
}

/*
 * @brief	System_Init
 * @param   none
 * @retval  none
 * @func	系统初始化
 */
void System_Init(void)
{
	uint8_t Tx_Frequency_Paramete;
	uint8_t TX_Vibration_Paramete;

	HAL_Delay(200);
	Mcu_GpioInit();
	AD9833_InitIo(AD9877_Ch_A);
	AD9833_InitIo(AD9877_Ch_B);

	STMFLASH_Read(FLASH_SLAVE_ADJUST,(uint16_t*)&FreqAdjustParam,2);
	STMFLASH_Read(FLASH_SLAVE_LOWFLG,(uint16_t*)&Low_Battery_Flg,1);
	STMFLASH_Read(FLASH_SLAVE_LOCKFLG ,(uint16_t*)&LockOn_ParamFlg,1);
	STMFLASH_Read(FLASH_SLAVE_WORKCOUNT,(uint16_t*)(&WorkCount),1);
	STMFLASH_Read(FLASH_SLAVE_SHUNTDOWM ,(uint16_t*)&PowerFlg,1);

	if(FreqAdjustParam[0] == 0xFFFF)
	{
		duty_cycle_count = 101;
	}else
	{
		duty_cycle_count = (uint8_t)FreqAdjustParam[0];
	}

	if(FreqAdjustParam[1] == 0xFFFF)
	{
		duty_tim = 5000;
	}else
	{
		duty_tim = FreqAdjustParam[1];
	}

	SetPWMDutyCycleAndFrequency(duty_tim);

	if(WorkCount == 0xFFFF)
	{
		WorkCount = 0;
	}else
	{
	}

	if(PowerFlg == 0x01)
	{
		Ultrasound_state = CLOSE_STATE;
	}else if(PowerFlg == 0xFFFF)
	{
		PowerFlg = 0;
	}else
	{
	}

	STMFLASH_Read(FLASH_SLAVE_CHANGEFLG,(uint16_t*)(Change_Paramete_Flg),3);
	if((Change_Paramete_Flg[0] == 0xdddd)&&(Change_Paramete_Flg[1] == 0xeeee)&&(Change_Paramete_Flg[2] == 0xffff))
	{
		STMFLASH_Read(FLASH_SLAVE_ULTRAPARAM,(uint16_t*)(&Ultrasound_Parem_Read_Buff),3);

		Frequency_Parameter_A = (uint8_t)Ultrasound_Parem_Read_Buff[0];
		Frequency_Parameter_B = (uint8_t)Ultrasound_Parem_Read_Buff[1];
		Vibration_Paramete = (uint8_t)Ultrasound_Parem_Read_Buff[2];
		/**硬件进行1/2的预分频 所以软件处理时要乘2**/
		u32_frequency_parameter = 2000*(Frequency_Parameter_A+ULTRA_BASE);
		AD9833_SetPara(AD9877_Ch_A,AD9833_REG_FREQ0,u32_frequency_parameter,AD9833_REG_PHASE1,2048,AD9833_OUT_TRIANGLE);

		u32_frequency_parameter = 2000*(Frequency_Parameter_B+ULTRA_BASE);
		AD9833_SetPara(AD9877_Ch_B,AD9833_REG_FREQ0,u32_frequency_parameter,AD9833_REG_PHASE1,2048,AD9833_OUT_TRIANGLE);

		//电机驱动频率72000000/(72 * 4167) ≈ 240Hz  驱动电压 = 3.3 * 55 / 100
		u32_vibration_parameter = Vibration_Paramete * MOTOR_BASE;

		Comd_funparameter_Frequency_txbuffer_A = Frequency_Parameter_A;
		Comd_funparameter_Frequency_txbuffer_B = Frequency_Parameter_B;
		Comd_funparameter_Vibration_txbuffer = Vibration_Paramete;
	}else if((Change_Paramete_Flg[0] == 0xffff)&&(Change_Paramete_Flg[1] == 0xffff)&&(Change_Paramete_Flg[2] == 0xffff))
	{
		Tx_Frequency_Paramete = 0X4A;
		TX_Vibration_Paramete = 0X0A;

		//if(Tx_Frequency_Paramete == 84)
		{
			HAL_Delay(100);
			u32_frequency_parameter = 2000*(Tx_Frequency_Paramete+ULTRA_BASE);
			if(u32_frequency_parameter == 1270000)
			{
				HAL_UART_Transmit(&huart1,&Tx_Frequency_Paramete, 1, HAL_MAX_DELAY);
			}
			AD9833_SetPara(AD9877_Ch_A,AD9833_REG_FREQ0,u32_frequency_parameter,AD9833_REG_PHASE1,2048,AD9833_OUT_TRIANGLE);
			AD9833_SetPara(AD9877_Ch_B,AD9833_REG_FREQ0,u32_frequency_parameter,AD9833_REG_PHASE1,2048,AD9833_OUT_TRIANGLE);
		}


		//if(TX_Vibration_Paramete == 0x0A)
		{
			HAL_Delay(100);
			u32_vibration_parameter = TX_Vibration_Paramete * MOTOR_BASE;
			if(u32_vibration_parameter == 1600)
			{

			}
		}

		Comd_funparameter_Frequency_txbuffer_A = Tx_Frequency_Paramete;
		Comd_funparameter_Frequency_txbuffer_B = Tx_Frequency_Paramete;
		Comd_funparameter_Vibration_txbuffer = TX_Vibration_Paramete;
	}
	ultrasound_standby_time = 0;
	battery_send_intervaltime = 0;
}

/*
 * @brief	Usart3_ReceiveData
 * @param   none
 * @retval  none
 * @func	串口接收数据处理
 */
void Usart3_ReceiveData(void)
{

	if(USART3_RX_FLAG)
	{
		USART3_RX_FLAG = 0;
		/**超声开启/停止指令**/
		if((USART3_RX_BUF[0] == 0xEE) && (USART3_RX_BUF[1] == 0xB5) && (USART3_RX_BUF[2] == 0x21) && (USART3_RX_BUF[12] == 0xFF) && (USART3_RX_BUF[13] == 0xFC) && (USART3_RX_BUF[14] == 0xFF) && (USART3_RX_BUF[15] == 0xFF))
		{
			if((USART3_RX_BUF[3] == 0x0) && (USART3_RX_BUF[4] == 0x0) && (USART3_RX_BUF[5] == 0x02) && (USART3_RX_BUF[6] == 0x01)) //超声开启
			{
				Ultrasound_state = WORK_STATE;
				HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
				DMA_USART3_Tx_Data(Cmd_GetTim, 6);
			}else if((USART3_RX_BUF[3] == 0x0) && (USART3_RX_BUF[4] == 0x0) && (USART3_RX_BUF[5] == 0x04) && (USART3_RX_BUF[6] == 0x00)) //超声停止
			{
				Ultrasound_state = STANDBY_STATE;

				HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);
			}else if(USART3_RX_BUF[3] == 0xBC) //超声频率设置
			{
				if(USART3_RX_BUF[6] == 0x10)
				{
					LockOn_ParamFlg = 0x01;
					STMFLASH_Write(FLASH_SLAVE_LOCKFLG,(uint16_t*)(&LockOn_ParamFlg),1);
				}else if((USART3_RX_BUF[6] == 0x01) && (LockOn_ParamFlg != 0x01))
				{
					Ultrasound_state = PUASE_STATE;
					//超声参数保存Flg位
					Change_Paramete_Flg[0] = 0xdddd;
					Change_Paramete_Flg[1] = 0xeeee;
					Change_Paramete_Flg[2] = 0xffff;
					STMFLASH_Write(FLASH_SLAVE_CHANGEFLG,(uint16_t*)(Change_Paramete_Flg),3);

					Ultrasound_Parem_Write_Buff[0] = (uint16_t)USART3_RX_BUF[4];
					Ultrasound_Parem_Write_Buff[1] = (uint16_t)USART3_RX_BUF[7];
					Ultrasound_Parem_Write_Buff[2] = (uint16_t)USART3_RX_BUF[5];

					STMFLASH_Write(FLASH_SLAVE_ULTRAPARAM,(uint16_t*)(&Ultrasound_Parem_Write_Buff),3);
					STMFLASH_Read(FLASH_SLAVE_ULTRAPARAM,(uint16_t*)(&Ultrasound_Parem_Read_Buff),3);

					if((Ultrasound_Parem_Write_Buff[0] == Ultrasound_Parem_Read_Buff[0]) && (Ultrasound_Parem_Write_Buff[1] == Ultrasound_Parem_Read_Buff[1])&&(Ultrasound_Parem_Write_Buff[2] == Ultrasound_Parem_Read_Buff[2]))
					{
						Mcu_GpioInit();

						Frequency_Parameter_A = (uint8_t)Ultrasound_Parem_Read_Buff[0];
						Frequency_Parameter_B = (uint8_t)Ultrasound_Parem_Read_Buff[1];
						Vibration_Paramete = (uint8_t)Ultrasound_Parem_Read_Buff[2];
						/**硬件进行1/2的预分频 所以软件处理时要乘2**/
						u32_frequency_parameter = 2000*(Frequency_Parameter_A+ULTRA_BASE);
						AD9833_SetPara(AD9877_Ch_A,AD9833_REG_FREQ0,u32_frequency_parameter,AD9833_REG_PHASE1,2048,AD9833_OUT_TRIANGLE);

						u32_frequency_parameter = 2000*(Frequency_Parameter_B+ULTRA_BASE);
						AD9833_SetPara(AD9877_Ch_B,AD9833_REG_FREQ0,u32_frequency_parameter,AD9833_REG_PHASE1,2048,AD9833_OUT_TRIANGLE);

						//电机驱动频率72000000/(72 * 4167) ≈ 240Hz  驱动电压 = 3.3 * 55 / 100
						u32_vibration_parameter = Vibration_Paramete * MOTOR_BASE;

						Comd_funparameter_txbuffer[2] = 0X21;
						Comd_funparameter_txbuffer[4] = Frequency_Parameter_A;
						Comd_funparameter_txbuffer[5] = Vibration_Paramete;
						Comd_funparameter_txbuffer[6] = Frequency_Parameter_B;

						Comd_funparameter_Frequency_txbuffer_A = Frequency_Parameter_A;
						Comd_funparameter_Frequency_txbuffer_B = Frequency_Parameter_B;
						Comd_funparameter_Vibration_txbuffer = Vibration_Paramete;

						DMA_USART3_Tx_Data(Comd_funparameter_txbuffer, 14);
						HAL_Delay(2);
						DMA_USART3_Tx_Data(Comd_funparameter_txbuffer, 14);

						HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);
					}
				}else
				{}

			}else if((USART3_RX_BUF[3] == 0xBD) && (USART3_RX_BUF[4] == 0xF9)) //超声占空比设置
			{
				duty_cycle_count = USART3_RX_BUF[5];
				duty_tim = (((uint16_t)USART3_RX_BUF[6] << 8) | (uint16_t)USART3_RX_BUF[7]);

				if(duty_cycle_count == 0x64)
				{
					duty_cycle_count = 0x65;
				}

				if((duty_cycle_count == (uint8_t)FreqAdjustParam[0]) && (duty_tim == FreqAdjustParam[1]))
				{

				}else
				{
					FreqAdjustParam[0] = (uint16_t)duty_cycle_count;
					FreqAdjustParam[1] = duty_tim;
					SetPWMDutyCycleAndFrequency(duty_tim);
					STMFLASH_Write(FLASH_SLAVE_ADJUST,FreqAdjustParam,2);
				}


			}else
			{
			}
		}

		/**进入管理员界面 系统挂起**/
		if((USART3_RX_BUF[0] == 0xEE) && (USART3_RX_BUF[1] == 0xB5) && (USART3_RX_BUF[2] == 0x11) && (USART3_RX_BUF[12] == 0xFF) && (USART3_RX_BUF[13] == 0xFC) && (USART3_RX_BUF[14] == 0xFF) && (USART3_RX_BUF[15] == 0xFF))
		{
			if((USART3_RX_BUF[3] == 0x0) && (USART3_RX_BUF[4] == 0x0) && (USART3_RX_BUF[5] == 0x00) && (USART3_RX_BUF[6] == 0x0B))   //进入管理员界面
			{
				ultrasound_standby_time = 0;
				Ultrasound_state = PUASE_STATE;

				Comd_funparameter_txbuffer[2] = 0x13;
				Comd_funparameter_txbuffer[4] = Comd_funparameter_Frequency_txbuffer_A;
				Comd_funparameter_txbuffer[5] = Comd_funparameter_Vibration_txbuffer;
				Comd_funparameter_txbuffer[6] = Comd_funparameter_Frequency_txbuffer_B;
				DMA_USART3_Tx_Data(Comd_funparameter_txbuffer, 14);
				HAL_Delay(2);
				DMA_USART3_Tx_Data(Comd_funparameter_txbuffer, 14);

				HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);
			}else if((USART3_RX_BUF[3] == 0x0) && (USART3_RX_BUF[4] == 0x0A) && (USART3_RX_BUF[5] == 0x00) && (USART3_RX_BUF[6] == 0x06)) //退出管理员界面
			{
				ultrasound_standby_time = 0;
				Ultrasound_state = STANDBY_STATE;

				HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);
			}
		}

		if((USART3_RX_BUF[0] == 0xEE) && (USART3_RX_BUF[1] == 0xF7) && (USART3_RX_BUF[9] == 0xFF) && (USART3_RX_BUF[10] == 0xFC) && (USART3_RX_BUF[11] == 0xFF) && (USART3_RX_BUF[12] == 0xFF))
		{
			Start_Time.Year = USART3_RX_BUF[2];
			Start_Time.Mon = USART3_RX_BUF[3];
			Start_Time.Day = USART3_RX_BUF[5];
			Start_Time.Hour = USART3_RX_BUF[6];
			Start_Time.Min = USART3_RX_BUF[7];
		}
	}
}


/*
 * @brief	Ultra_FourStates
 * @param   none
 * @retval  none
 * @func	超声工作状态，输出脉冲信号
 */
void Ultra_FourStates(void)
{
	static uint8_t BatteryChFlg = 0;
	static uint32_t old_tick = 0;
	uint32_t new_tick = 0;

	if(process_flag == 1)
	{
		process_flag = 0;

		if(battery_level_state < boost_battery_level1)
		{
#ifdef CHARGE_WORK

#else
			Ultrasound_state = CHARGE_STATE;
#endif

			new_tick = HAL_GetTick();
			if((new_tick - old_tick) > 1000)
			{
				old_tick = new_tick;
				BatteryChFlg = 1;
			}

			if((BatteryChFlg == 1) && (PowerFlg == 0))
			{
				PowerFlg = 1;
				STMFLASH_Write(FLASH_SLAVE_SHUNTDOWM,&PowerFlg,1);
			}
		}else
		{
			if(BatteryChFlg == 0x01)
			{
				Ultrasound_state = CLOSE_STATE;
			}
			new_tick = HAL_GetTick();
			old_tick = new_tick;
		}
		if((receive_battery_state_data & 0x80) == 0x80)
		{
			if(battery_adc_value1 <= 3200)   //自动关机3.2V
			{
				Ultrasound_state = CLOSE_STATE;
				FinRes = Vol_Low;
			}else
			{
				if((Ultrasound_state == WORK_STATE)||(Ultrasound_state == PUASE_STATE)||(Ultrasound_state == CLOSE_STATE))
				{

				}else
				{
					Ultrasound_state = STANDBY_STATE;
				}
			}
		}

	}

	switch(Ultrasound_state)
	{
		//充电状态
		case CHARGE_STATE:
			{
				ultrasound_standby_time = 0;
				if(WorkStartFlg == 1)
				{
					WorkFinishFlg = 1;
				}
				Mcu_GpioInit();
			}
				break;
		//待机状态
		case STANDBY_STATE:
			{
				if(WorkStartFlg == 1)
				{
					WorkFinishFlg = 1;
				}

				HAL_GPIO_WritePin(LTDCDC_EN_GPIO_Port, LTDCDC_EN_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(MOTOR_GATE_GPIO_Port, MOTOR_GATE_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(WAVE_A_EN_GPIO_Port, WAVE_A_EN_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(WAVE_B_EN_GPIO_Port, WAVE_B_EN_Pin, GPIO_PIN_RESET);

				__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, 0);
				__HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_4, 0);	//PWM_Ultra_A

				if(ultrasound_standby_time >= 4500)
				{
					ultrasound_standby_time = 0;
					if(battery_level_state < boost_battery_level1)
					{

					}else
					{
						Ultrasound_state = CLOSE_STATE;
					}
				}
			}
				break;
		//挂起状态
		case PUASE_STATE:
			{
				if(WorkStartFlg == 1)
				{
					WorkFinishFlg = 1;
				}

				HAL_GPIO_WritePin(LTDCDC_EN_GPIO_Port, LTDCDC_EN_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(MOTOR_GATE_GPIO_Port, MOTOR_GATE_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(WAVE_A_EN_GPIO_Port, WAVE_A_EN_Pin, GPIO_PIN_RESET);
				HAL_GPIO_WritePin(WAVE_B_EN_GPIO_Port, WAVE_B_EN_Pin, GPIO_PIN_RESET);

				__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, 0);
				__HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_4, 0);	//PWM_Ultra_A

				if(ultrasound_standby_time >= 60000)
				{
					ultrasound_standby_time = 0;
					if(battery_level_state < boost_battery_level1)
					{

					}else
					{
						Ultrasound_state = CLOSE_STATE;
					}
				}
			}
				break;
		//工作状态
		case WORK_STATE:
			{
				WorkStartFlg = 1;
				ultrasound_standby_time = 0;

				HAL_GPIO_WritePin(LTDCDC_EN_GPIO_Port, LTDCDC_EN_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(MPDCDC_EN_GPIO_Port, MPDCDC_EN_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(MOTOR_GATE_GPIO_Port, MOTOR_GATE_Pin, GPIO_PIN_SET);

				HAL_GPIO_WritePin(WAVE_A_EN_GPIO_Port, WAVE_A_EN_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(WAVE_B_EN_GPIO_Port, WAVE_B_EN_Pin, GPIO_PIN_SET);

				if(Motor_Worktime < (duty_cycle_count * 100))
				{
					__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, u32_vibration_parameter);
					__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, 0);
				}else if((Motor_Worktime >= (duty_cycle_count * 100)) && (Motor_Worktime < (100 * 100)))
				{
					__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_2, 0);
					__HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, 0);
				}else
				{
					Motor_Worktime = 0;
				}
			}
				break;
		//关机状态
		case CLOSE_STATE:
			{
				if(WorkStartFlg == 1)
				{
					WorkFinishFlg = 1;
				}

				if((ShuntDowmCount > 130) && (PowerFlg == 1))
				{
					PowerFlg = 0;
					STMFLASH_Write(FLASH_SLAVE_SHUNTDOWM,(uint16_t*)&PowerFlg,1);
				}

				HAL_GPIO_WritePin(KEY_CONTROL_GPIO_Port, KEY_CONTROL_Pin, GPIO_PIN_SET);
				HAL_GPIO_WritePin(CONTROL_CLOSE_GPIO_Port, CONTROL_CLOSE_Pin, GPIO_PIN_SET);
			}
				break;
		default :

				break;
	}
}

/*
 * @brief	Log_Writer2Flash
 * @param   none
 * @retval  none
 * @func	将单次使用的档位、时间信息记录到Flash中
 */
void Log_Writer2Flash(void)
{
	uint32_t work_time_temp = 0;
	uint32_t addr_offset = 0;
	//正常的开始工作到工作结束，记录一次
	if((WorkStartFlg == 1) && (WorkFinishFlg == 1))
	{
		WorkCount++;
		if(duty_cycle_count == 25)
		{
			//异常结束的原因
			WorkParm[0] = (0x10 | FinRes);
		}else
		{
			WorkParm[0] = (0x20 | FinRes);
		}
		work_time_temp = WorkTime / 100;
		WorkParm[1] = (uint8_t)((work_time_temp & 0xFF00) >> 8);
		WorkParm[2] = (uint8_t)(work_time_temp & 0x00FF);
		WorkParm[3] = Start_Time.Year;
		WorkParm[4] = Start_Time.Mon;
		WorkParm[5] = Start_Time.Day;
		WorkParm[6] = Start_Time.Hour;
		WorkParm[7] = Start_Time.Min;

		//记录到8600行之后回滚到0,目前Flash最大记录8600行
		addr_offset =  FLASH_SLAVE_LOGIN + ((WorkCount % 3800) * 8);

		STMFLASH_Write(FLASH_SLAVE_WORKCOUNT,(uint16_t*)&WorkCount,1);
		STMFLASH_Write(addr_offset ,(uint16_t*)WorkParm,4);

		WorkTime = 0;
		WorkStartFlg = 0;
		WorkFinishFlg = 0;
	}
}

/*
 * @brief	Warning_LowBattery
 * @param   none
 * @retval  none
 * @func	低电量报警，电量低于2格报警，只有当电量大于3格或充电20分钟解锁
 */
void Warning_LowBattery(void)
{
	//从工作到结束，并且电量低于2格时低电量关机
	if((CompleteFlg == 1) && ((battery_level_state == boost_battery_level2) || (battery_level_state == boost_battery_level1)))
	{
		if(Low_Battery_Flg == 1)
		{

		}else
		{
			Low_Battery_Flg = 1;
			STMFLASH_Write(FLASH_SLAVE_LOWFLG,(uint16_t*)&Low_Battery_Flg,1);
		}

	}

	if(battery_level_state > boost_battery_level3)
	{
		Low_Battery_Flg = 0;
	}

	if(Charge_Time > 120000)
	{
		Charge_Time = 0;
		Low_Battery_Flg = 0;

		STMFLASH_Write(FLASH_SLAVE_LOWFLG,(uint16_t*)&Low_Battery_Flg,1);
	}
}

/*
 * @brief
 * @param 	pulsetime:脉冲周期时间
 * @param	dutycycle:脉冲占空比
 * @retval	none
 * @func	调节脉冲时间和占空比
 */

#define TIM3_CLK 72000000
#define MIN_OUTPUT_FREQ 2
#define MAX_OUTPUT_FREQ 10000

static void SetPWMDutyCycleAndFrequency(uint32_t pulsetime)
{
	uint32_t input_freq = 0;
	uint32_t out_prescaler_value = 0;
	uint32_t out_count_value = 0;

	input_freq = 10000 / pulsetime;

	if((input_freq > MAX_OUTPUT_FREQ) || (input_freq < MIN_OUTPUT_FREQ))
	{
		return ;
	}

	if((input_freq >= 20) && (input_freq <= 10000))
	{
		out_prescaler_value = 720-1;
		out_count_value = 1000000 / input_freq;
	}else if((input_freq >= 2) && (input_freq < 20))
	{
		out_prescaler_value = 7200-1;
		out_count_value = 100000 / input_freq;
	}else
	{
		return ;
	}

	// 停止定时器
	HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_3);
	HAL_TIM_Base_Stop(&htim4);

	__HAL_TIM_SET_PRESCALER(&htim4, out_prescaler_value);
	__HAL_TIM_SetAutoreload(&htim4, out_count_value);
	duty_count = out_count_value;

	__HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_4, 0);											//PWM_Ultra_A
	__HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_3, (duty_count * duty_cycle_count / 100)); 	// 假设使用通道1	//PWM_Ultra_B
	//__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_3, 100);

    // 更新并启动定时器
	HAL_TIM_Base_Start(&htim4);
    //HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
}

