/*
 * sram.h
 *
 *  Created on: Jun 14, 2024
 *      Author: hardik
 */

#ifndef INC_SRAM_H_
#define INC_SRAM_H_

#include "main.h"
#include "string.h"
//#include "stm32g0xx_hal.h"
// SRAM Variables
//uint8_t SRAM_READ  = 0x03;
//uint8_t SRAM_WRITE = 0x02;

#define SRAM_READ 0x03
#define SRAM_WRITE 0x02

extern I2C_HandleTypeDef hi2c1;

extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

// Function to enable the chip select (CS)
void SRAM_Select(void);
// Function to disable the chip select (CS)
void SRAM_Deselect(void);
// Function to write data to SRAM
void SRAM_Write(uint32_t address, uint8_t *data, uint16_t length);
// Function to read data from SRAM
void SRAM_Read(uint32_t address, uint8_t *data, uint16_t length);


#endif /* INC_SRAM_H_ */
