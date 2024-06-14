/*
 * sram.c
 *
 *  Created on: Jun 14, 2024
 *      Author: hardik
 */
#include "sram.h"


// Function to enable the chip select (CS)
void SRAM_Select(void) {
    //HAL_GPIO_WritePin(SRAM_CS1_GPIO_Port, SRAM_CS1_Pin, GPIO_PIN_RESET);	// Pull the NSS Pin Low, to start SPI Communication
    HAL_GPIO_WritePin(SRAM_CS2_GPIO_Port, SRAM_CS2_Pin, GPIO_PIN_RESET);	// Pull the NSS Pin Low, to start SPI Communication
}

// Function to disable the chip select (CS)
void SRAM_Deselect(void) {
	//HAL_GPIO_WritePin(SRAM_CS1_GPIO_Port, SRAM_CS1_Pin, GPIO_PIN_SET);	// Setting the NSS Pin High, by default (NSS Pin is Active Low)
	HAL_GPIO_WritePin(SRAM_CS2_GPIO_Port, SRAM_CS2_Pin, GPIO_PIN_SET);	// Setting the NSS Pin High, by default (NSS Pin is Active Low)
}

// Function to write data to SRAM
void SRAM_Write(uint32_t address, uint8_t *data, uint16_t length) {
    uint8_t addr_buf[3];
    addr_buf[0] = (address >> 16) & 0xFF;
    addr_buf[1] = (address >> 8) & 0xFF;
    addr_buf[2] = address & 0xFF;

    SRAM_Select();

    //uint8_t command = SRAM_WRITE;
    HAL_SPI_Transmit(&hspi1, SRAM_WRITE, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi1, addr_buf, 3, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi1, data, length, HAL_MAX_DELAY);

    SRAM_Deselect();
    /*char buffer[50];
    snprintf(buffer, sizeof(buffer), "data: %ld\n", pressure_lo);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);*/
}

// Function to read data from SRAM
void SRAM_Read(uint32_t address, uint8_t *data, uint16_t length) {
    uint8_t addr_buf[3];
    addr_buf[0] = (address >> 16) & 0xFF;
    addr_buf[1] = (address >> 8) & 0xFF;
    addr_buf[2] = address & 0xFF;

    SRAM_Select();

    //uint8_t command = SRAM_READ;
    HAL_SPI_Transmit(&hspi1, SRAM_READ, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi1, addr_buf, 3, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, data, length, HAL_MAX_DELAY);

    SRAM_Deselect();
    char buffer[length];
    snprintf(buffer, sizeof(buffer), "data: %ld\n", data);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
}



