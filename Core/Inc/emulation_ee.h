/*
********************************************************************************
* Filename      : emulation_ee.h
* Version       : V1.00
* Created on    : 2023.10.19
* Programmer(s) : LLJ
********************************************************************************
*/

#ifndef INC_EMULATION_EE_H_
#define INC_EMULATION_EE_H_

#include "main.h"

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define __IO    volatile
typedef __IO uint16_t vu16;


#define STM32_FLASH_SIZE 	128
    #if     STM32_FLASH_SIZE < 256
    #define STM_SECTOR_SIZE     1024
    #else
    #define STM_SECTOR_SIZE	    2048
    #endif
#define STM32_FLASH_BASE    0x08000000
#define FLASH_SAVE_ADDR     STM32_FLASH_BASE+0xF000
#define STM32_FLASH_WREN 	1
#define FLASH_WAITETIME  	50000


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
#define FLASH_SLAVE_ADJUST	   0x0800EC00
#define FLASH_SLAVE_CHANGEFLG  0x0800F000
#define FLASH_SLAVE_LOWFLG	   0x0800F400
#define FLASH_SLAVE_LOCKFLG    0x0800F800
#define FLASH_SLAVE_ULTRAPARAM 0x0800FC00
#define FLASH_SLAVE_SHUNTDOWM  0x08010000
#define FLASH_SLAVE_RECEIVE	   0x08010400
#define FLASH_SLAVE_ENCRYPT    0x08010800
#define FLASH_SLAVE_WORKCOUNT  0x08010C00
#define FLASH_SLAVE_LOGIN	   0x08010C08


u8 STMFLASH_GetStatus(void);
u8 STMFLASH_WaitDone(u16 time);
u8 STMFLASH_ErasePage(u32 paddr);
u8 STMFLASH_WriteHalfWord(u32 faddr, u16 dat);
u16 STMFLASH_ReadHalfWord(u32 faddr);
void STMFLASH_WriteLenByte(u32 WriteAddr,u32 DataToWrite,u16 Len);
u32 STMFLASH_ReadLenByte(u32 ReadAddr,u16 Len);
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);
void Flash_PageErase(uint32_t PageAddress);


#endif /* INC_EMULATION_EE_H_ */
