/*
 * psense.c
 *
 *  Created on: Jun 14, 2024
 *      Author: hardik
 */
#include "psense.h"

void Read_Pressure_Sensor(I2C_HandleTypeDef *hi2c1, uint16_t *pressure)
{
  uint8_t pressure_data[3];
  HAL_StatusTypeDef status;

    status = HAL_I2C_Mem_Read(hi2c1, MPM3801A_ADDRESS, 0x00, I2C_MEMADD_SIZE_8BIT, pressure_data, 3, HAL_MAX_DELAY);
    if (status == HAL_OK)
    {
      uint32_t raw_pressure = (pressure_data[0] << 16) | (pressure_data[1] << 8) | pressure_data[2];
      *pressure = ((float)raw_pressure) / 4096.0f;  // Convert raw data to pressure value
      int32_t pressure_lo = ((int32_t)(pressure_data[0] << 16 | pressure_data[1] << 8 | pressure_data[2])) >> 4;
      char buffer[50];
      snprintf(buffer, sizeof(buffer), "Pressure: %ld Pa\n", pressure_lo);
      HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
    }
    else
    {
      char buffer[10] = "Failed";
      HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
    }

    //return status;
  //HAL_I2C_Mem_Read(&hi2c1, 0xC0, 0x00, I2C_MEMADD_SIZE_8BIT, pressure_data, 3, HAL_MAX_DELAY);

  // Convert and print the pressure
  //int32_t pressure_lo = ((int32_t)(pressure_data[0] << 16 | pressure_data[1] << 8 | pressure_data[2])) >> 4;
  //char buffer[50];
  //snprintf(buffer, sizeof(buffer), "Pressure: %ld Pa\n", pressure_lo);
  //HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
  //return status;
}
