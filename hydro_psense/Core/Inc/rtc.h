/*
 * rtc.h
 *
 *  Created on: Jun 14, 2024
 *      Author: hardi
 */

#ifndef INC_RTC_H_
#define INC_RTC_H_

#include "main.h"

#define DS3231_ADDRESS 0x68 << 1

extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;

void Read_RTC_Time(I2C_HandleTypeDef *hi2c1, uint8_t *data);

void ConvertToReadableTime(uint8_t* buffer);


#endif /* INC_RTC_H_ */
