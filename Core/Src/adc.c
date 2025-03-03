/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    adc.c
  * @brief   This file provides code for the configuration
  *          of the ADC instances.
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
/* Includes ------------------------------------------------------------------*/
#include "adc.h"

/* USER CODE BEGIN 0 */
#include "usart.h"

uint16_t bat_temp_val = 0;
uint16_t battery_adc_value1 = 3500;
uint32_t battery_adc_buff[15];
uint8_t SampleStarFlg = 0;
uint8_t Send_DataBuf[128] = {0};

static uint16_t SampleVolFilter(uint32_t* buf,uint8_t len);
/* USER CODE END 0 */

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

/* ADC1 init function */
void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_13CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */
    /* ADC1 clock enable */
    __HAL_RCC_ADC1_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**ADC1 GPIO Configuration
    PB1     ------> ADC1_IN9
    */
    GPIO_InitStruct.Pin = VBAT_ADC_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(VBAT_ADC_GPIO_Port, &GPIO_InitStruct);

    /* ADC1 DMA Init */
    /* ADC1 Init */
    hdma_adc1.Instance = DMA1_Channel1;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(adcHandle,DMA_Handle,hdma_adc1);

  /* USER CODE BEGIN ADC1_MspInit 1 */

  /* USER CODE END ADC1_MspInit 1 */
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC1_CLK_DISABLE();

    /**ADC1 GPIO Configuration
    PB1     ------> ADC1_IN9
    */
    HAL_GPIO_DeInit(VBAT_ADC_GPIO_Port, VBAT_ADC_Pin);

    /* ADC1 DMA DeInit */
    HAL_DMA_DeInit(adcHandle->DMA_Handle);
  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
/*!
 * @brief   : ADC1_init
 * @param	: void
 * @retval  : none
 * @funct   : ADC采样初始化，采样校准，开启DMA
 */
void ADC1_Init(void)
{
	HAL_ADCEx_Calibration_Start(&hadc1);
	//HAL_ADC_Start(&hadc1);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&bat_temp_val, 1);
}

/*!
 * @brief  :ADC1_GetBatteryVol
 * @param  :void
 * @retval :none
 * @func   :获取电池电压，对电压进行滤波处理，标定不�??????????要？
 */
void ADC1_GetBatteryVol(void)
{
	float battery_vol = 0.0f;
	uint32_t new_tick = 0;
	static uint8_t SampleCount = 0;
	static uint32_t old_tick = 0;

	//等待DMA空闲执行下一次采�?
	if(__HAL_DMA_GET_FLAG(&hadc1,DMA_FLAG_TC1) == SET)
	{
	   __HAL_DMA_CLEAR_FLAG(&hadc1,DMA_FLAG_TC1);
	   HAL_ADC_Start_DMA(&hadc1, (uint32_t *)&bat_temp_val, 1);
	}
	battery_vol = (float)bat_temp_val * 3300.0f / 4095;
	//采样采用的分压电�?3:2左右
	battery_vol = battery_vol / 0.6285f;

	if(SampleStarFlg == 0)
	{
		//第一次采样完全填�?
		for(SampleCount = 0;SampleCount < 15;SampleCount++)
		{
			battery_adc_buff[SampleCount] = (uint32_t)battery_vol;
		}
		SampleStarFlg = 1;
	}else
	{
		battery_adc_buff[SampleCount] = (uint32_t)battery_vol;
		SampleCount++;
		if(SampleCount >= 15)
		{
			SampleCount = 0;
		}
	}

	battery_adc_value1  = SampleVolFilter(battery_adc_buff,15);
	new_tick = HAL_GetTick();
	if((new_tick - old_tick) > 500)
	{
		old_tick = new_tick;
		printf("Vol=%d\n",battery_adc_value1);
	}
}

/*
 * @brief	:SampleVolFilter
 * @param   :buf  电压存储缓存
 * @param   :len  数据长度
 * @retval  :vol  滤波后的电压
 * @func	:去掉�??大最小�?�取平均值，防止采样误差
 */

static uint16_t SampleVolFilter(uint32_t* buf,uint8_t len)
{
	uint32_t vol = 0,vol_sum = 0;
	uint32_t vol_temp = 0;
	uint8_t tag_i = 0,tag_j = 0;

	for(tag_i = 0;tag_i < (len - 1);tag_i++)
	{
		for(tag_j = 0;tag_j < (len - tag_i - 1);tag_j++)
		{
			if(buf[tag_j] > buf[tag_j+1])
			{
				vol_temp = buf[tag_j];
				buf[tag_j] = buf[tag_j+1];
				buf[tag_j+1] = vol_temp;
			}
		}
	}

	for(tag_i = 0;tag_i < len-2;tag_i++)
	{
		vol_sum += buf[tag_i+1];
	}
	vol = (uint16_t)(vol_sum/(len-2));

	return vol;
}
/* USER CODE END 1 */
