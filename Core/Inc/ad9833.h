/*
********************************************************************************
* Filename      : battery.h
* Version       : V1.00
* Created on    : 2023.10.19
* Programmer(s) : LLJ
********************************************************************************
*/

#ifndef INC_AD9833_H_
#define INC_AD9833_H_


#include <math.h>
#include <stdbool.h>
#include "main.h"

#define AD9833PORT 	GPIOB
#define AD9833DATA 	A_SCLK_Pin
#define AD9833SCK 	A_SDATA_Pin
#define AD9833SS 	A_FSYNC_Pin
#define AD9833_SDATA_0() 	HAL_GPIO_WritePin(GPIOD,AD9833DATA,GPIO_PIN_RESET);
#define AD9833_SDATA_1() 	HAL_GPIO_WritePin(GPIOD,AD9833DATA,GPIO_PIN_SET);
#define AD9833_SCLK_0()	 	HAL_GPIO_WritePin(AD9833PORT,AD9833SCK,GPIO_PIN_RESET);
#define AD9833_SCLK_1()	 	HAL_GPIO_WritePin(AD9833PORT,AD9833SCK,GPIO_PIN_SET);
#define AD9833_FSYNC_0()	HAL_GPIO_WritePin(AD9833PORT,AD9833SS,GPIO_PIN_RESET);
#define AD9833_FSYNC_1()	HAL_GPIO_WritePin(AD9833PORT,AD9833SS,GPIO_PIN_SET);
#define AD9833PORT_B 		GPIOC
#define AD9833DATA_B 		B_SCLK_Pin
#define AD9833SCK_B 		B_SDATA_Pin
#define AD9833SS_B 			B_FSYNC_Pin
#define AD9833_SDATA_0_B() 		HAL_GPIO_WritePin(AD9833PORT_B,AD9833DATA_B,GPIO_PIN_RESET);
#define AD9833_SDATA_1_B() 		HAL_GPIO_WritePin(AD9833PORT_B,AD9833DATA_B,GPIO_PIN_SET);
#define AD9833_SCLK_0_B()	 	HAL_GPIO_WritePin(AD9833PORT_B,AD9833SCK_B,GPIO_PIN_RESET);
#define AD9833_SCLK_1_B()	 	HAL_GPIO_WritePin(AD9833PORT_B,AD9833SCK_B,GPIO_PIN_SET);
#define AD9833_FSYNC_0_B()		HAL_GPIO_WritePin(AD9833PORT_B,AD9833SS_B,GPIO_PIN_RESET);
#define AD9833_FSYNC_1_B()		HAL_GPIO_WritePin(AD9833PORT_B,AD9833SS_B,GPIO_PIN_SET);
#define AD9833_OUT_SINUS    ((0 << 5) | (0 << 1) | (0 << 3))
#define AD9833_OUT_TRIANGLE ((0 << 5) | (1 << 1) | (0 << 3))
#define AD9833_OUT_MSB      ((1 << 5) | (0 << 1) | (1 << 3))
#define AD9833_OUT_MSB2     ((1 << 5) | (0 << 1) | (0 << 3))
#define AD9833_REG_CMD      (0 << 14)
#define AD9833_REG_FREQ0    (1 << 14)
#define AD9833_REG_FREQ1    (2 << 14)
#define AD9833_REG_PHASE0   (6 << 13)
#define AD9833_REG_PHASE1   (7 << 13)
#define AD9833_B28          (1 << 13)
#define AD9833_HLB          (1 << 12)
#define AD9833_FSEL0        (0 << 11)
#define AD9833_FSEL1        (1 << 11)
#define AD9833_PSEL0        (0 << 10)
#define AD9833_PSEL1        (1 << 10)
#define AD9833_PIN_SW       (1 << 9)
#define AD9833_RESET        (1 << 8)
#define AD9833_CLEAR_RESET  (0 << 8)
#define AD9833_SLEEP1       (1 << 7)
#define AD9833_SLEEP12      (1 << 6)
#define AD9833_OPBITEN      (1 << 5)
#define AD9833_SIGN_PIB     (1 << 4)
#define AD9833_DIV2         (1 << 3)
#define AD9833_MODE         (1 << 1)

typedef enum
{
	AD9877_Ch_A,
	AD9877_Ch_B,
	AD9877_MaxCh,
}AD9833_MaxTable;

// ------------------ Functions  ---------------------
void AD9833_Delay(void);
void AD9833_InitIo(uint8_t ch);
void AD9833_SetPara(uint8_t ch,uint32_t Freq_SFR,double Freq,uint32_t Phase_SFR,uint32_t Phase,uint32_t WaveMode);

#endif /* INC_AD9833_H_ */
