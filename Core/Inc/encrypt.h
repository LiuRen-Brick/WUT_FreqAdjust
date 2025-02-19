/*
 * dev_encrypt.h
 *
 *  Created on: Mar 20, 2024
 *      Author: 15754
 */

#ifndef ENCRYPT_DEV_ENCRYPT_H_
#define ENCRYPT_DEV_ENCRYPT_H_

#include "main.h"

typedef union
{
	uint16_t uid_key_16u[6];
	uint32_t uid_key_32u[3];
}UIDKEY;

void Encrypt_ApplicaStar(void);

#endif /* ENCRYPT_DEV_ENCRYPT_H_ */
