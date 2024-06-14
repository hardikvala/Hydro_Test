/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include <stdio.h>
#include "string.h"
#include "ssd1306.h"
#include "ssd1306_tests.h"
#include "ssd1306_conf.h"
#include "ssd1306_fonts.h"
#include <stdlib.h> // For itoa function
#include "Bitmap.h";
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_MQTT_RSP_LENGTH 500 // bytes
/* MPM3801A I2C address */
//#define MPM3801A_ADDRESS 0x28
#define DS3231_ADDRESS 0x68 << 1


/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
volatile unsigned int rx_buf_indx = 0;
volatile char ch = '\0';
volatile char rx_buf[MAX_MQTT_RSP_LENGTH] = {'\0'};

uint8_t time_data[7];
uint16_t pressure = 0;

//Temp value
char units[]="Units";
char bar[]="-> 1)Bar";
char psi[]="   2)PSI";
char kg[]= "   3)Kg/cm";
char sq[]=    "2";
char hmode[]="Mode";
char hmode1[]="-> 1)Auto";
char hmode2[]="   2)Manual";
char tank[]= "Equipment No. = 12354684116";

// AT Commands
uint8_t AT[] = "AT\r";
uint8_t ATE[] = "ATE1\r";
uint8_t ATQ[] = "ATQ0\r";
uint8_t ATI[] = "ATI\r";
uint8_t SET_AT_CREG[] = "AT+CREG=1\r";
uint8_t GET_AT_CREG[] = "AT+CREG?\r";
//uint8_t SET_AT_CGACT[] = "AT+CGACT=1\r";  // NOTE: Sending this returns CME ERROR: 129
uint8_t GET_AT_CGACT[] = "AT+CGACT?\r";
uint8_t *SET_AT_HUBBLEREG = "";
uint8_t GET_AT_HUBBLEREG[] = "AT+HUBBLEREG?\r";
//uint8_t *AT_MQTTCREATE = "";
//uint8_t AT_MQTTCREATE[] = "AT+MQTTCREATE=\"broker.mqtt.cool\",1883,\"DEF_DEVICE\",250,0,0\r";
uint8_t AT_MQTTCREATE[100];
//uint8_t *AT_MQTTCONN = "";
uint8_t AT_MQTTCONN[50];


// Variable related to MQTT connections
const char *mqtt_broker = "broker.mqtt.cool";
const unsigned short int mqtt_port = 1883;
//const char *device_id = "DEF_DEVICE";
const char *device_id = "TEST0";

unsigned short int mqtt_client_id = 0;     // this will be auto assigned and can be parsed from the response of AT+CGACT?
const char *mqtt_username  = "";
const char *mqtt_password  = "";

const char *hubble_account_id  = "";
const char *hubble_plan_id  = "";


char *tmp_ptr, *tmp_ptr2;
char conn_id[10];

unsigned int num;


// SRAM Variables
//uint8_t SRAM_READ  = 0x03;
//uint8_t SRAM_WRITE = 0x02;
//uint8_t RDMR_SRAM[2]  = {0x05, 0x0};
//uint8_t WRMR_SRAM  = 0x01;
//uint8_t SRAM_ReadData[260] = {0x03, 0x00, 0x00, 0x00};		// 1st index = command to read. 2nd, 3rd and 4th forms 24bit long address
//uint8_t SRAM_WriteData[260] = {0x02, 0x00, 0x00, 0x00};		// 1st index = command to write. 2nd, 3rd and 4th forms 24bit long address

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI2_Init(void);
/* USER CODE BEGIN PFP */
void Read_RTC_Time(I2C_HandleTypeDef *hi2c1, uint8_t *data);
void ConvertToReadableTime(uint8_t* buffer);
//void Read_Pressure_Sensor(I2C_HandleTypeDef *hi2c1, uint16_t *pressure);

void LTE_c16qs_Init();
void LTE_c16qs_Reset();
void LTE_c16qs_TurnOff();

signed short int connect_mqtt_network(char *host, unsigned short int port, char *client_id, unsigned short int keep_alive);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE BEGIN 0 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
	if(huart->Instance == USART2){
		rx_buf[rx_buf_indx] = ch;
		rx_buf_indx++;

		/*if(rx_buf_indx >= MAX_MQTT_RSP_LENGTH-1){
			HAL_UART_Transmit(&huart1,(uint8_t *)rx_buf, sizeof(rx_buf), 100);
			memset(rx_buf, 0, sizeof(rx_buf));
			rx_buf_indx = 0;
		}*/
		HAL_UART_Receive_IT(&huart2, (uint8_t*)&ch, 1);
	}
}


// MQTT Functions
signed short int connect_mqtt_network(char *host, unsigned short int port, char *client_id, unsigned short int keep_alive){
		memset(rx_buf, '\0', sizeof(rx_buf));
		rx_buf_indx = 0;

		HAL_UART_Transmit(&huart1,(uint8_t *)ATE, sizeof(ATE), 10);  // In PROTO, AT commands should be sent on huart2
		//while(!strstr(buf, "OK") || !strstr(buf, "ERROR")){}
		while(!strstr(rx_buf, "OK\r\n")){}
		//while(!strstr(buf, "OK")){}
		HAL_UART_Transmit(&huart2,(uint8_t *)rx_buf, strlen(rx_buf), 10);    // In PROTO, this should be sent on huart1 = DBG_UART. if required


		memset(rx_buf, '\0', sizeof(rx_buf));
		rx_buf_indx = 0;
		//HAL_UART_Transmit(&huart2,(uint8_t *)ATQ, sizeof(ATI), 10);  // In PROTO, AT commands should be sent on huart2
		HAL_UART_Transmit(&huart1,(uint8_t *)ATQ, sizeof(ATI), 10);  // In PROTO, AT commands should be sent on huart2
		//while(!strstr(buf, "OK") || !strstr(buf, "ERROR")){}
		while(!strstr(rx_buf, "OK\r\n")){}
		//while(!strstr(buf, "OK")){}
		HAL_UART_Transmit(&huart2,(uint8_t *)rx_buf, strlen(rx_buf), 10);    // In PROTO, this should be sent on huart1 = DBG_UART. if required

		memset(rx_buf, '\0', sizeof(rx_buf));
		rx_buf_indx = 0;
		//HAL_UART_Transmit(&huart2,(uint8_t *)ATI, sizeof(ATQ), 10);  // In PROTO, AT commands should be sent on huart2
		HAL_UART_Transmit(&huart1,(uint8_t *)ATI, sizeof(ATQ), 10);  // In PROTO, AT commands should be sent on huart2
		//while(!strstr(buf, "OK") || !strstr(buf, "ERROR")){}
		while(!strstr(rx_buf, "OK\r\n")){}
		//while(!strstr(buf, "OK")){}
		HAL_UART_Transmit(&huart2,(uint8_t *)rx_buf, strlen(rx_buf), 100);    // In PROTO, this should be sent on huart1 = DBG_UART. if required

		memset(rx_buf, '\0', sizeof(rx_buf));
		rx_buf_indx = 0;
		HAL_UART_Transmit(&huart1,(uint8_t *)SET_AT_CREG, sizeof(SET_AT_CREG), 10);  // In PROTO, AT commands should be sent on huart2
		while(!(strstr(rx_buf, "OK\r\n") || strstr(rx_buf, "ERROR\r\n"))){}
		if(strstr(rx_buf, "ERROR\r\n")){
			HAL_UART_Transmit(&huart2,(uint8_t *)rx_buf, strlen(rx_buf), 10);
			return -1;
		}
		HAL_UART_Transmit(&huart2,(uint8_t *)rx_buf, strlen(rx_buf), 10);

		// get AT+CREG? and compare response with expectations  // TODO: what are the expectations?
		memset(rx_buf, '\0', sizeof(rx_buf));
		rx_buf_indx = 0;
		HAL_UART_Transmit(&huart1,(uint8_t *)GET_AT_CREG, strlen(GET_AT_CREG), 10);  // In PROTO, AT commands should be sent on huart2
		while(!(strstr(rx_buf, "OK\r\n") || strstr(rx_buf, "ERROR\r\n"))){}
		if(strstr(rx_buf, "ERROR\r\n")){
			HAL_UART_Transmit(&huart2,(uint8_t *)rx_buf, strlen(rx_buf), 10);
			return -1;
		}
		HAL_UART_Transmit(&huart2,(uint8_t *)rx_buf, strlen(rx_buf), 100);

		// get AT+CGACT? and check that the 'state' = 1 active)
		memset(rx_buf, '\0', sizeof(rx_buf));
		rx_buf_indx = 0;
		HAL_UART_Transmit(&huart1,(uint8_t *)GET_AT_CGACT, sizeof(GET_AT_CGACT), 100);  // In PROTO, AT commands should be sent on huart2
		while(!(strstr(rx_buf, "OK\r\n") || strstr(rx_buf, "ERROR\r\n"))){}
		if(strstr(rx_buf, "ERROR\r\n")){
			HAL_UART_Transmit(&huart2,(uint8_t *)rx_buf, strlen(rx_buf), 100);
			return -1;
		}
		HAL_UART_Transmit(&huart2,(uint8_t *)rx_buf, strlen(rx_buf), 100);

		// get AT+HUBBLEREG?
		memset(rx_buf, '\0', sizeof(rx_buf));
		rx_buf_indx = 0;
		HAL_UART_Transmit(&huart1,(uint8_t *)GET_AT_HUBBLEREG, sizeof(GET_AT_HUBBLEREG), 100);  // In PROTO, AT commands should be sent on huart2
		while(!(strstr(rx_buf, "OK\r\n") || strstr(rx_buf, "ERROR\r\n"))){}
		if(strstr(rx_buf, "ERROR\r\n") || strstr(rx_buf, "UNREGISTERED")){
			HAL_UART_Transmit(&huart2,(uint8_t *)rx_buf, strlen(rx_buf), 100);
			return -1;
		}
		HAL_UART_Transmit(&huart2,(uint8_t *)rx_buf, strlen(rx_buf), 100);

		// set AT+MQTTCREATE
		memset(rx_buf, '\0', sizeof(rx_buf));
		rx_buf_indx = 0;
		sprintf(AT_MQTTCREATE, "AT+MQTTCREATE=\"%s\",%0d,\"%s\",%0d,0,0\r", mqtt_broker, mqtt_port, device_id, 250);
		//strcpy(AT_MQTTCREATE, "AT+MQTTCREATE=\"broker.mqtt.cool\",1883,\"DEF_DEVICE\",250,0,0\r");
		//HAL_UART_Transmit(&huart2,(uint8_t *)AT_MQTTCREATE, sizeof(AT_MQTTCREATE), 200);
		HAL_UART_Transmit(&huart1,(uint8_t *)AT_MQTTCREATE, sizeof(AT_MQTTCREATE), 100);
		while(!(strstr(rx_buf, "OK\r\n") || strstr(rx_buf, "ERROR\r\n"))){}
		if(strstr(rx_buf, "ERROR\r\n")){
			HAL_UART_Transmit(&huart2,(uint8_t *)rx_buf, strlen(rx_buf), 100);
			return -1;
		}
		HAL_UART_Transmit(&huart2,(uint8_t *)rx_buf, strlen(rx_buf), 100);

		tmp_ptr = strrchr(rx_buf, ':');
		HAL_UART_Transmit(&huart2,(uint8_t *)tmp_ptr, strlen(tmp_ptr), 100);

		tmp_ptr2 = strchr(rx_buf, ':');
		HAL_UART_Transmit(&huart2,(uint8_t *)tmp_ptr2, strlen(tmp_ptr2), 100);

		strncpy(conn_id, tmp_ptr2+2, 1);  // TODO: may need to update this logic, so that it can parse longer digits od client_id also
		conn_id[1] = '\0';
		HAL_UART_Transmit(&huart2,(uint8_t *)conn_id, strlen(conn_id), 100);

		mqtt_client_id = atoi(conn_id);

		// AT+MQTTCONN
		memset(rx_buf, '\0', sizeof(rx_buf));
		rx_buf_indx = 0;
		sprintf(AT_MQTTCONN, "AT+MQTTCONN=%d\r", mqtt_client_id);
		//HAL_UART_Transmit(&huart2,(uint8_t *)AT_MQTTCONN, sizeof(AT_MQTTCONN), 100);
		HAL_UART_Transmit(&huart1,(uint8_t *)AT_MQTTCONN, strlen(AT_MQTTCONN), 100);
		while(!(strstr(rx_buf, "OK\r\n") || strstr(rx_buf, "ERROR\r\n"))){}
		if(strstr(rx_buf, "ERROR\r\n")){
			HAL_UART_Transmit(&huart2,(uint8_t *)rx_buf, strlen(rx_buf), 100);
			return -1;
		}
		HAL_UART_Transmit(&huart2,(uint8_t *)rx_buf, strlen(rx_buf), 100);
		if(strstr(rx_buf, "CONNECTED")){
			return 1;
		}
		else{
			return 0;
		}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_SPI2_Init();
  /* USER CODE BEGIN 2 */

  HAL_UART_Receive_IT(&huart2, (uint8_t*)&ch, 1);
  strcpy(rx_buf, "starting...\r\n");
  HAL_UART_Transmit(&huart1,(uint8_t *)rx_buf, strlen(rx_buf), 100);

  // Initialize Cavli Module
  //LTE_c16qs_Init();
  GSM_Init();
   // Send AT command
      GSM_SendCommand("ATE1\r\n");
      HAL_Delay(1000);  // Delay for 1 second
      GSM_ReadResponse();
      GSM_SendCommand("AT\r\n");
      HAL_Delay(1000);  // Delay for 1 second
      GSM_ReadResponse();
      GSM_SendCommand("AT+CIMI\r\n");
      HAL_Delay(1000);  // Delay for 1 second
      GSM_ReadResponse();

/*
  // Reading RDMR from SRAM1 //
  HAL_GPIO_WritePin(SRAM_CS1_GPIO_Port, SRAM_CS1_Pin, GPIO_PIN_SET);	// Setting the NSS Pin High, by default (NSS Pin is Active Low)
  HAL_GPIO_WritePin(SRAM_CS1_GPIO_Port, SRAM_CS1_Pin, GPIO_PIN_RESET);	// Pull the NSS Pin Low, to start SPI Communication
  HAL_SPI_Receive(&hspi1, RDMR_SRAM, sizeof(RDMR_SRAM), 20);
  HAL_GPIO_WritePin(SRAM_CS1_GPIO_Port, SRAM_CS1_Pin, GPIO_PIN_SET);	// Push the NSS Pin High, to stop SPI Communication
  HAL_Delay(100);

  for(int i=4; i<260; i++){
	  SRAM_WriteData[i] = i-4; // storing 256Bytes into an array. This will be used later to Write an entire page
  }

  HAL_GPIO_WritePin(SRAM_CS1_GPIO_Port, SRAM_CS1_Pin, GPIO_PIN_RESET);	// Pull the NSS Pin Low, to start SPI Communication
  HAL_SPI_Transmit(&hspi1, SRAM_WriteData, sizeof(SRAM_WriteData)-4, 200);
  HAL_GPIO_WritePin(SRAM_CS1_GPIO_Port, SRAM_CS1_Pin, GPIO_PIN_SET);	// Push the NSS Pin High, to stop SPI Communication
  HAL_Delay(5000);

  for(int i=4; i<260; i++){
	  SRAM_ReadData[i] = 0xff; // storing 256Bytes into an array. This will be used later to Write an entire page
  }

  HAL_GPIO_WritePin(SRAM_CS1_GPIO_Port, SRAM_CS1_Pin, GPIO_PIN_RESET);	// Pull the NSS Pin Low, to start SPI Communication
  HAL_SPI_Receive(&hspi1, SRAM_ReadData, sizeof(SRAM_ReadData)-4, 200);
  HAL_GPIO_WritePin(SRAM_CS1_GPIO_Port, SRAM_CS1_Pin, GPIO_PIN_SET);	// Push the NSS Pin High, to stop SPI Communication
  HAL_Delay(5000);
*/
  // SRAM TEST
  uint8_t write_data[] = {0x11, 0x22, 0x33, 0x44};
  uint8_t read_data[4];

  SRAM_Write(0x000000, write_data, sizeof(write_data));
  SRAM_Read(0x000000, read_data, sizeof(read_data));



  ssd1306_Init();
  ssd1306_DrawRectangle(0,0,127,63,White);

  ssd1306_SetCursor(47, 3);
  ssd1306_WriteString(units,Font_6x8, White);
  ssd1306_Line(1, 11, 127, 11, White);			//LINE And Box

  ssd1306_SetCursor(3, 14);                     //Bar
  ssd1306_WriteString(bar,Font_6x8, White);

  ssd1306_SetCursor(3, 24);                     //PSI
  ssd1306_WriteString(psi,Font_6x8, White);


  ssd1306_SetCursor(4, 34);                     //Kg/cm2
  ssd1306_WriteString(kg,Font_6x8, White);

  ssd1306_DrawPixel(63,33, White);

  ssd1306_SetCursor(63, 30);                     //Square
  ssd1306_WriteString(sq,Font_6x8, White);

  ssd1306_UpdateScreen();




  // power on Cavli Module
  //HAL_GPIO_WritePin(GPIOA, GSM_PWR_CKT_Pin, GPIO_PIN_SET);   // Enable for PROTO
/*
  if(connect_mqtt_network(mqtt_broker, mqtt_port, device_id, 250) == 1){
	  HAL_UART_Transmit(&huart2,(uint8_t *)"Network Connected", strlen("Network Connected"), 100);
  }
  else{
	  HAL_UART_Transmit(&huart2,(uint8_t *)"No Network Connection", strlen("No Network Connection"), 100);
  }
*/

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  Read_RTC_Time(&hi2c1, &time_data);
	  ConvertToReadableTime(time_data);
	  Read_Pressure_Sensor(&hi2c1, &pressure);
	  HAL_Delay(1000);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00303D5B;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_1LINE;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart1, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart1, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart2, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart2, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, SRAM_CS1_Pin|SRAM_CS2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_3_GPIO_Port, LED_3_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED_2_Pin|LED_1_Pin|SWO_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GSM_RESET_Pin|GSM_WAKEUP_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GSM_PWR_CKT_GPIO_Port, GSM_PWR_CKT_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, OLED_CS_Pin|OLED_Res_Pin|OLED_DC_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : SRAM_CS1_Pin SRAM_CS2_Pin */
  GPIO_InitStruct.Pin = SRAM_CS1_Pin|SRAM_CS2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_3_Pin LED_2_Pin LED_1_Pin SWO_Pin */
  GPIO_InitStruct.Pin = LED_3_Pin|LED_2_Pin|LED_1_Pin|SWO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : SW_3_Pin */
  GPIO_InitStruct.Pin = SW_3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SW_3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : RTC_INT_Pin */
  GPIO_InitStruct.Pin = RTC_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(RTC_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : GSM_RESET_Pin GSM_WAKEUP_Pin OLED_CS_Pin OLED_Res_Pin
                           OLED_DC_Pin */
  GPIO_InitStruct.Pin = GSM_RESET_Pin|GSM_WAKEUP_Pin|OLED_CS_Pin|OLED_Res_Pin
                          |OLED_DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : GSM_PWR_CKT_Pin */
  GPIO_InitStruct.Pin = GSM_PWR_CKT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GSM_PWR_CKT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SW_2_Pin */
  GPIO_InitStruct.Pin = SW_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SW_2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Batt_v_Pin */
  GPIO_InitStruct.Pin = Batt_v_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Batt_v_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SW_1_Pin */
  GPIO_InitStruct.Pin = SW_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SW_1_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/**
  * @brief Initialize the DS3231 RTC
  * @param None
  * @retval None
  */
void DS3231_Init(void)
{
  // Typically, no initialization is required for DS3231
}

/**
  * @brief Initialize the MPM3801A Pressure Sensor
  * @param None
  * @retval None
  */
void MPM3801A_Init(void)
{
  // Typically, no initialization is required for MPM3801A
}

/**
  * @brief Read RTC time and print it via USART1
  * @param None
  * @retval None
  */
//HAL_StatusTypeDef Read_RTC_Time(I2C_HandleTypeDef *hi2c1, uint8_t *data)
/*void Read_RTC_Time(I2C_HandleTypeDef *hi2c1, uint8_t *data)
{
  //uint8_t rtc_data[7];
  HAL_StatusTypeDef status;
  status = HAL_I2C_Mem_Read(hi2c1, DS3231_ADDRESS, 0x00, I2C_MEMADD_SIZE_8BIT, data, 7, HAL_MAX_DELAY);

  // Convert and print the time
  char buffer[50];
  snprintf(buffer, sizeof(buffer), "Time: %02x:%02x:%02x\n", data[2], data[1], data[0]);
  HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
  //return status;
}

void ConvertToReadableTime(uint8_t* buffer) {
  uint8_t seconds = ((buffer[0] >> 4) * 10) + (buffer[0] & 0x0F);
  uint8_t minutes = ((buffer[1] >> 4) * 10) + (buffer[1] & 0x0F);
  uint8_t hours = ((buffer[2] >> 4) * 10) + (buffer[2] & 0x0F);
  uint8_t day = ((buffer[3] >> 4) * 10) + (buffer[3] & 0x0F);
  uint8_t date = ((buffer[4] >> 4) * 10) + (buffer[4] & 0x0F);
  uint8_t month = ((buffer[5] >> 4) * 10) + (buffer[5] & 0x0F);
  uint8_t year = ((buffer[6] >> 4) * 10) + (buffer[6] & 0x0F);

  printf("Time: %02d:%02d:%02d Date: %02d-%02d-20%02d Day: %d\n", hours, minutes, seconds, date, month, year, day);
}*/
/**
  * @brief Read pressure sensor data and print it via USART1
  * @param None
  * @retval None
  */
//HAL_StatusTypeDef _Read_Pressure_Sensor(I2C_HandleTypeDef *hi2c1, float *pressure)
//void Read_RTC_Time(I2C_HandleTypeDef *hi2c1, uint8_t *data)
//void ConvertToReadableTime(uint8_t* buffer)
/*void Read_Pressure_Sensor(I2C_HandleTypeDef *hi2c1, uint16_t *pressure)
{
  uint8_t pressure_data[3];
  HAL_StatusTypeDef status;

    status = HAL_I2C_Mem_Read(hi2c1, MPM3801A_ADDRESS << 1, 0x00, I2C_MEMADD_SIZE_8BIT, pressure_data, 3, HAL_MAX_DELAY);
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
*/

/*void GSM_SendCommand(const char *cmd)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)cmd, strlen(cmd), HAL_MAX_DELAY);
}

void GSM_ReadResponse(char *buffer, uint16_t size)
{
    HAL_UART_Receive(&huart2, (uint8_t*)buffer, size, HAL_MAX_DELAY);
}
*/

/**
  * @brief Initialize the Cavli C16QS LTE module
  * @param None
  * @retval None
  */
/*void LTE_c16qs_Init(void)
{

	// Power on the GSM module
	HAL_GPIO_WritePin(GSM_PWR_CKT_GPIO_Port, GSM_PWR_CKT_Pin, GPIO_PIN_RESET);
	HAL_Delay(1000); // Wait for the module to power on

	// Wake up the GSM module
	HAL_GPIO_WritePin(GSM_WAKEUP_GPIO_Port, GSM_WAKEUP_Pin, GPIO_PIN_RESET);
	HAL_Delay(500);
	HAL_GPIO_WritePin(GSM_WAKEUP_GPIO_Port, GSM_WAKEUP_Pin, GPIO_PIN_SET);

	// Reset the GSM module
	HAL_GPIO_WritePin(GSM_RESET_GPIO_Port, GSM_RESET_Pin, GPIO_PIN_SET);
	HAL_Delay(500);
	HAL_GPIO_WritePin(GSM_RESET_GPIO_Port, GSM_RESET_Pin, GPIO_PIN_RESET);
	HAL_Delay(2000);
	// 1. Drive GSM_PWR_CKT LOW, to power on the Cavli module
	//    GSM_PWR_CKT is configured with OUTPUT_LEVEL=HIGH. So, by default it is HIGH
    //HAL_GPIO_WritePin(GSM_PWR_CKT_GPIO_Port, GSM_PWR_CKT_Pin, GPIO_PIN_RESET);
	/*HAL_GPIO_WritePin(GSM_PWR_CKT_GPIO_Port, GSM_PWR_CKT_Pin, 1);
    HAL_Delay(500);
	// 2. wake it up by driving GSM_WAKEUP to LOW for at least 500ms (so, GSM_WAKEUP should be HIGH in normal conditions)
	//HAL_GPIO_WritePin(GSM_WAKEUP_GPIO_Port, GSM_WAKEUP_Pin, GPIO_PIN_RESET);
	//HAL_Delay(500);  // TODO: need to fine-tune this delay
	//HAL_GPIO_WritePin(GSM_WAKEUP_GPIO_Port, GSM_WAKEUP_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GSM_WAKEUP_GPIO_Port, GSM_WAKEUP_Pin, 1);
	HAL_Delay(2000);  // TODO: need to fine-tune this delay
    */

	// Send a command to the GSM module
/*	GSM_SendCommand("ATI\r\n");

	// Read the response
	char response[100];
	GSM_ReadResponse(response, sizeof(response));
	while(!strstr(response, "OK\r\n")){
		GSM_ReadResponse(response, sizeof(response));
		snprintf(response, sizeof(response), "data: %ld\n", response);
	    HAL_UART_Transmit(&huart1,(uint8_t *)response, strlen(response), 10);
	}
	//snprintf(response, sizeof(response), "data: %s\n", response);
	HAL_UART_Transmit(&huart1,(uint8_t *)response, strlen(response), 10);
	// Print the response (depends on your debug method, e.g., to a serial monitor)
	printf("Response: %s\n", response);*/
	// For debug
	/*HAL_UART_Transmit(&huart1,(uint8_t *)rx_buf, strlen(rx_buf), 10);

	memset(rx_buf, '\0', sizeof(rx_buf));
	rx_buf_indx = 0;
	HAL_UART_Transmit(&huart1,(uint8_t *)AT, sizeof(AT), 10);  // for debug only
	HAL_UART_Transmit(&huart2,(uint8_t *)AT, sizeof(AT), 10);
	while(!strstr(rx_buf, "OK\r\n")){}
	HAL_UART_Transmit(&huart1,(uint8_t *)rx_buf, strlen(rx_buf), 10);
}
*/

/**
  * @brief Reset the Cavli C16QS LTE module
  * @param None
  * @retval None
  */
void LTE_c16qs_Reset(void)
{
	// (to reset Cavli module, drive GSM_RESET to Low for 500ms (so, GSM_RESET should be HIGH in normal conditions))
	HAL_GPIO_WritePin(GSM_RESET_GPIO_Port, GSM_RESET_Pin, GPIO_PIN_RESET);
	HAL_Delay(500);  // TODO: need to fine-tune this delay
	HAL_GPIO_WritePin(GSM_RESET_GPIO_Port, GSM_RESET_Pin, GPIO_PIN_SET);
	HAL_Delay(500);  // TODO: need to fine-tune this delay
}


/**
  * @brief Turn-off the Cavli C16QS LTE module
  * @param None
  * @retval None
  */
void LTE_c16qs_TurnOff(void)
{
	HAL_GPIO_WritePin(GSM_PWR_CKT_GPIO_Port, GSM_PWR_CKT_Pin, GPIO_PIN_SET);
}
/*
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

    uint8_t command = SRAM_WRITE;
    HAL_SPI_Transmit(&hspi1, &command, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi1, addr_buf, 3, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi1, data, length, HAL_MAX_DELAY);

    SRAM_Deselect();
    /*char buffer[50];
    snprintf(buffer, sizeof(buffer), "data: %ld\n", pressure_lo);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
}

// Function to read data from SRAM
void SRAM_Read(uint32_t address, uint8_t *data, uint16_t length) {
    uint8_t addr_buf[3];
    addr_buf[0] = (address >> 16) & 0xFF;
    addr_buf[1] = (address >> 8) & 0xFF;
    addr_buf[2] = address & 0xFF;

    SRAM_Select();

    uint8_t command = SRAM_READ;
    HAL_SPI_Transmit(&hspi1, &command, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi1, addr_buf, 3, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, data, length, HAL_MAX_DELAY);

    SRAM_Deselect();
    char buffer[length];
    snprintf(buffer, sizeof(buffer), "data: %ld\n", data);
    HAL_UART_Transmit(&huart1, (uint8_t*)buffer, strlen(buffer), HAL_MAX_DELAY);
}
*/
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
