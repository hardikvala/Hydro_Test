/*
 * gsm.c
 *
 *  Created on: Jun 14, 2024
 *      Author: hardik
 */

#include "gsm.h"

extern UART_HandleTypeDef huart2;

// Buffer for receiving data
static uint8_t rx_buffer[100];

void GSM_Init(void) {
    // Initialize GSM module
    /*HAL_GPIO_WritePin(GSM_PWR_CKT_GPIO_Port, GSM_PWR_CKT_Pin, GPIO_PIN_RESET);  // Set GSM_PWR_CKT_Pin to low
    HAL_GPIO_WritePin(GSM_RESET_GPIO_Port, GSM_RESET_Pin, GPIO_PIN_SET);      // Disable Reset
    HAL_GPIO_WritePin(GSM_WAKEUP_GPIO_Port, GSM_WAKEUP_Pin, GPIO_PIN_RESET);   // Wakeup GSM
    HAL_Delay(5000);
    HAL_GPIO_WritePin(GSM_WAKEUP_GPIO_Port, GSM_WAKEUP_Pin, GPIO_PIN_SET);
    */
    // Initialize GSM module
    HAL_GPIO_WritePin(GPIOA, GSM_PWR_CKT_Pin, GPIO_PIN_RESET);  // Set GSM_PWR_CKT_Pin to low
    HAL_GPIO_WritePin(GPIOD, GSM_RESET_Pin, GPIO_PIN_SET);      // Disable Reset
    HAL_GPIO_WritePin(GPIOD, GSM_WAKEUP_Pin, GPIO_PIN_RESET);   // Wakeup GSM
    HAL_Delay(5000);
    HAL_GPIO_WritePin(GPIOD, GSM_WAKEUP_Pin, GPIO_PIN_SET);
}

void GSM_SendCommand(const char *command) {
    HAL_UART_Transmit(&huart2, (uint8_t*)command, strlen(command), HAL_MAX_DELAY);
}


void GSM_ReadResponse(void) {
    // Clear the buffer
    memset(rx_buffer, 0, sizeof(rx_buffer));

    // Receive response from GSM module
    HAL_UART_Receive(&huart2, rx_buffer, sizeof(rx_buffer) - 1, 5000);

    // Clean the response
    GSM_CleanResponse(rx_buffer);

    // Debug: Transmit the cleaned response back via UART2
    HAL_UART_Transmit(&huart2, rx_buffer, strlen((const char*)rx_buffer), HAL_MAX_DELAY);
}

void GSM_CleanResponse(uint8_t *response) {
    uint8_t *src = response;
    uint8_t *dst = response;

    while (*src) {
        if (*src == '^' || *src == '[' || *src == 'A') {
            // Skip unwanted characters
            src++;
        } else {
            // Copy character to the destination
            *dst++ = *src++;
        }
    }
    // Null-terminate the cleaned string
    *dst = '\0';
}

