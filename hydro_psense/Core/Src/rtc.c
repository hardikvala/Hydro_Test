/*
 * rtc.c
 *
 *  Created on: Jun 14, 2024
 *      Author: hardik
 */
#include "rtc.h"

void Read_RTC_Time(I2C_HandleTypeDef *hi2c1, uint8_t *data){
  //uint8_t rtc_data[7];
  HAL_StatusTypeDef status;
  status = HAL_I2C_Mem_Read(hi2c1, DS3231_ADDRESS, 0x00, I2C_MEMADD_SIZE_8BIT, data, 7, HAL_MAX_DELAY);

  // Convert and print the time
  char buffer[50];
  snprintf(buffer, sizeof(buffer), "Time: %02x:%02x:%02x\n", data[2], data[1], data[0]);
  HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
  //return status;
}

void ConvertToReadableTime(uint8_t* buffer){
  uint8_t seconds = ((buffer[0] >> 4) * 10) + (buffer[0] & 0x0F);
  uint8_t minutes = ((buffer[1] >> 4) * 10) + (buffer[1] & 0x0F);
  uint8_t hours = ((buffer[2] >> 4) * 10) + (buffer[2] & 0x0F);
  uint8_t day = ((buffer[3] >> 4) * 10) + (buffer[3] & 0x0F);
  uint8_t date = ((buffer[4] >> 4) * 10) + (buffer[4] & 0x0F);
  uint8_t month = ((buffer[5] >> 4) * 10) + (buffer[5] & 0x0F);
  uint8_t year = ((buffer[6] >> 4) * 10) + (buffer[6] & 0x0F);

  printf("Time: %02d:%02d:%02d Date: %02d-%02d-20%02d Day: %d\n", hours, minutes, seconds, date, month, year, day);
}
