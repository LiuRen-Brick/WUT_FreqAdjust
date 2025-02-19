/*
 * dev_encrypt.c
 *
 *  Created on: Mar 20, 2024
 *      Author: 15754
 */
#include "encrypt.h"
#include "emulation_ee.h"

#define ENCRYPTION_KEY	0xC3C3C3C3

UIDKEY Uid_Key_Read;
uint8_t write_uid_flg = 0;
uint8_t read_uid_flg = 0;
uint32_t UID[3] = {0};
static uint32_t UID_Key[3] = {0};

/*
 * @brief	Encrypt_UidGet
 * @param   none
 * @retval  none
 * @func	获取单片机的UID号
 */
static void Encrypt_UidGet(void)
{
	UID[0] = HAL_GetUIDw0();
	UID[1] = HAL_GetUIDw1();
	UID[2] = HAL_GetUIDw2();
}

/*
 * @brief	Encrypt_UidSecretKey
 * @param   none
 * @retval  none
 * @func	获取UID密钥
 */
static void Encrypt_UidSecretKey(void)
{
	UID_Key[0] = UID[0] ^ ENCRYPTION_KEY;
	UID_Key[1] = UID[1] ^ ENCRYPTION_KEY;
	UID_Key[2] = UID[2] ^ ENCRYPTION_KEY;
}

/*
 * @brief	Encrypt_ApplicaStar
 * @param   none
 * @retval  none
 * @func	程序启动，上电读取到的密钥与存在Flash的密钥对比
 */
void Encrypt_ApplicaStar(void)
{
	Encrypt_UidGet();
	Encrypt_UidSecretKey();

	STMFLASH_Read(FLASH_SLAVE_ENCRYPT,Uid_Key_Read.uid_key_16u,6);

	if((Uid_Key_Read.uid_key_32u[0] == 0xFFFFFFFF) && \
	   (Uid_Key_Read.uid_key_32u[1] == 0xFFFFFFFF) && \
	   (Uid_Key_Read.uid_key_32u[2] == 0xFFFFFFFF))
	{
		//第一次上电程序保存密钥
		write_uid_flg = 1;
		Uid_Key_Read.uid_key_32u[0] = UID[0] ^ ENCRYPTION_KEY;
		Uid_Key_Read.uid_key_32u[1] = UID[1] ^ ENCRYPTION_KEY;
		Uid_Key_Read.uid_key_32u[2] = (UID[2] ^ ENCRYPTION_KEY);
		STMFLASH_Write(FLASH_SLAVE_ENCRYPT,Uid_Key_Read.uid_key_16u,6);
	}else if((UID_Key[0] != Uid_Key_Read.uid_key_32u[0]) || (UID_Key[1] != Uid_Key_Read.uid_key_32u[1]) || (UID_Key[2] != Uid_Key_Read.uid_key_32u[2]))
	{
		//获取的密钥与Flash不匹配，程序进入死循环
		while(1);
	}else
	{

	}
}
