/*
 * psense.h
 *
 *  Created on: Jun 14, 2024
 *      Author: hardik
 */

#ifndef INC_PSENSE_H_
#define INC_PSENSE_H_

#include "main.h"

#define MPM3801A_ADDRESS 0x28 << 1
extern I2C_HandleTypeDef hi2c1;

extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

void Read_Pressure_Sensor(I2C_HandleTypeDef *hi2c1, uint16_t *pressure);

#endif /* INC_PSENSE_H_ */
