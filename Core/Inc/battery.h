/*
********************************************************************************
* Filename      : battery.h
* Version       : V1.00
* Created on    : 2023.10.19
* Programmer(s) : LLJ
********************************************************************************
*/
#ifndef INC_BATTERY_H_
#define INC_BATTERY_H_
#include "main.h"

enum battery_level
{
	null,
	charge_battery_level1 = 1,
	charge_battery_level2 = 2,
	charge_battery_level3 = 3,
	charge_battery_level4 = 4,
	charge_battery_level5 = 5,
	boost_battery_level1 = 6,
	boost_battery_level2 = 7,
	boost_battery_level3 = 8,
	boost_battery_level4 = 9,
	boost_battery_level5 = 10,
};

void Battery_LevelReceive(void);
void Battery_LevelSend(void);

#endif /* INC_BATTERY_H_ */
