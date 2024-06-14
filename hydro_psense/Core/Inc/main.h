/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SRAM_SCK_Pin GPIO_PIN_5
#define SRAM_SCK_GPIO_Port GPIOA
#define SRAM_SO_Pin GPIO_PIN_6
#define SRAM_SO_GPIO_Port GPIOA
#define SRAM_SI_Pin GPIO_PIN_7
#define SRAM_SI_GPIO_Port GPIOA
#define SRAM_CS1_Pin GPIO_PIN_4
#define SRAM_CS1_GPIO_Port GPIOC
#define SRAM_CS2_Pin GPIO_PIN_5
#define SRAM_CS2_GPIO_Port GPIOC
#define LED_3_Pin GPIO_PIN_0
#define LED_3_GPIO_Port GPIOB
#define LED_2_Pin GPIO_PIN_1
#define LED_2_GPIO_Port GPIOB
#define LED_1_Pin GPIO_PIN_2
#define LED_1_GPIO_Port GPIOB
#define SW_3_Pin GPIO_PIN_11
#define SW_3_GPIO_Port GPIOB
#define RTC_INT_Pin GPIO_PIN_8
#define RTC_INT_GPIO_Port GPIOA
#define GSM_RESET_Pin GPIO_PIN_8
#define GSM_RESET_GPIO_Port GPIOD
#define GSM_WAKEUP_Pin GPIO_PIN_9
#define GSM_WAKEUP_GPIO_Port GPIOD
#define GSM_PWR_CKT_Pin GPIO_PIN_10
#define GSM_PWR_CKT_GPIO_Port GPIOA
#define SW_2_Pin GPIO_PIN_8
#define SW_2_GPIO_Port GPIOC
#define Batt_v_Pin GPIO_PIN_9
#define Batt_v_GPIO_Port GPIOC
#define SW_1_Pin GPIO_PIN_3
#define SW_1_GPIO_Port GPIOD
#define OLED_CS_Pin GPIO_PIN_4
#define OLED_CS_GPIO_Port GPIOD
#define OLED_Res_Pin GPIO_PIN_5
#define OLED_Res_GPIO_Port GPIOD
#define OLED_DC_Pin GPIO_PIN_6
#define OLED_DC_GPIO_Port GPIOD
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
#define DBG_TX_Pin GPIO_PIN_6
#define DBG_TX_GPIO_Port GPIOB
#define DBG_RX_Pin GPIO_PIN_7
#define DBG_RX_GPIO_Port GPIOB
#define RTC_SCL_Pin GPIO_PIN_8
#define RTC_SCL_GPIO_Port GPIOB
#define RTC_SDA_Pin GPIO_PIN_9
#define RTC_SDA_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
