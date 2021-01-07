/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "mavlink_v2/standard/mavlink.h"
#include "ssd1306/ssd1306.h"
#include "ssd1306/fonts.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_rx;

/* USER CODE BEGIN PV */
uint32_t adc_raw[2]; // ADC reading - IN1, Vbat
float vbat; // battery voltage
uint8_t uart1RX[1]; // mavlink
uint8_t Tx2Data[64]; // SIM800l
uint8_t Rx2Data[64]; // SIM800l

mavlink_system_t mavlink_system = {
	1, 	 // System ID (1-255)
	158    // Component ID (a MAV_COMPONENT value)
};

mavlink_status_t status;
mavlink_message_t msg;

bool sendTelemetry;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM3_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// switch onboard blue LED on and off
static void LED_Blink(uint32_t Hdelay, uint32_t Ldelay)
{
	HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_RESET);
	HAL_Delay(Hdelay - 1);
	HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_SET);
	HAL_Delay(Ldelay - 1);
}

// show message on OLED display
static void display(char *str)
{
	char delim[] = "\r\n";
	char *ptr = strtok(str, delim);
	SSD1306_Clear();
	u_int16_t y = 0;
	while (ptr!=NULL)
	{
		SSD1306_GotoXY(0, y);
		SSD1306_Puts(ptr, &Font_7x10, 1);
		ptr = strtok(NULL, delim);
		y += 12;
	}
	SSD1306_UpdateScreen();
}

static void displayBatteryVoltage() {
	uint8_t text[20];
	sprintf((char *)&text, "%.2fV", vbat);
	SSD1306_Clear();
	SSD1306_GotoXY(22, 12);
	SSD1306_Puts((char *)&text, &Font_16x26, 1);
	SSD1306_UpdateScreen();
	if (vbat < 3.5)
	{
		sprintf((char *)&text, "LOW BATTERY");
		SSD1306_GotoXY(0, 40);
		SSD1306_Puts((char *)&text, &Font_11x18, 1);
		SSD1306_UpdateScreen();
	}
}

/*
static void displayScroll()
{
  SSD1306_Scrolldiagright(0x0F, 0x0F);
}
*/

// send command to SIM800L, return response
static char* sim800(char* cmd)
{
	strcpy((char*)Tx2Data, cmd);
	strcpy((char*)Tx2Data + strlen(cmd), "\n\r");
	HAL_UART_Transmit(&huart2, Tx2Data, strlen((char*)Tx2Data), 200);
	memset(Rx2Data, 0x00, sizeof(Rx2Data));
	HAL_UART_Receive(&huart2, Rx2Data, sizeof(Rx2Data), 1000);
	return (char*) Rx2Data;
}

static bool displaySim800error(char* cmd, char* result)
{
  char msg[80];
  sprintf(msg, "%s\r\nERROR\r\n%s", cmd, result);
  display(msg);
  HAL_Delay(5000);
}

// send command to SIM800L, check response
static bool sim800check(char* cmd, char* expectedResult)
{
  char* res = sim800(cmd);
  if (!strcmp(res, expectedResult))
    return true;

  displaySim800error(cmd, res);
  return false;
}

/*
static char* sim800display(char* cmd)
{
	display(cmd);
  char* res = sim800(cmd);
	display(res);
	LED_Blink(5, 100);
	HAL_Delay(1000);
}

static void sim800info()
{
  sim800display("ATI");    // Display Product Identification Information
	sim800display("AT+CSQ"); // Signal Quality Report
	sim800display("AT+CGATT?");  // Attach or Detach from GPRS Service - Check if the MS is connected to the GPRS network 
}
*/

static bool sim800connect()
{
  // initial configuration - set parameters and save - run once for new SIM800L
  /*
  sim800("ATE0"); // Set Command Echo Mode OFF
  sim800("ATV0"); // Set TA Response Format - result codes
  sim800("AT&W"); // Save
  */



  // sim800("AT");
  if (!sim800check("AT", "0\r\n")) return false;
  // Set APN 
  if (!sim800check("AT+CSTT=\"TM\"", "0\r\n")) return false;
  // Bring up wireless connection with GPRS or CSD
  if (!sim800check("AT+CIICR", "0\r\n")) return false;
  // Get local IP address

  char* res = sim800("AT+CIFSR"); // if no error - response start from "\r\n", then ip address
  if (strncmp(res, "\r\n", 2))
  {
    displaySim800error("AT+CIFSR", res);
    return false;    
  }
  
  // Start Up TCP Connection
  if (!sim800check("AT+CIPSTART=\"TCP\",\"mail-verif.com\",20300", "0\r\n")) return false;
  return true; 
}

static bool sim800send(char* msg)
{
  char cmd[22];
  sprintf((char *)&cmd, "AT+CIPSEND=%d", strlen(msg));
  char* res = sim800(cmd);
  if (!strcmp(res, "\r\r\n> "))
  {
    res = sim800(msg);
    return (!strcmp(res, "\r\nSEND OK\r\n"));
  }
  return false;
}

static void sim800disconnect()
{
  // Close TCP Connection - ignore error
  sim800check("AT+CIPCLOSE", "\r\nCLOSE OK\r\n");
  // Deactivate GPRS PDP Context
  sim800check("AT+CIPSHUT", "\r\nSHUT OK\r\n");
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
  MX_DMA_Init();
  MX_TIM3_Init();
  MX_ADC1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  HAL_Delay(50); // wait at least 20ms for SSD1306
  SSD1306_Init();
  display("Hello STM32");

  HAL_TIM_Base_Start_IT(&htim3);
  HAL_UART_Receive_DMA(&huart1, uart1RX, sizeof(uart1RX));
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    // test
    display("AT");
    if (sim800check("AT", "0\r\n")) {
      display("OK");
    }
    HAL_Delay(1000);

    if (sendTelemetry)
    {
      display("Sending Telemetry");
      HAL_Delay(500);

      if (sim800connect())
      {
        display("Connect OK");
        HAL_Delay(1000);
        display("Send");
        if (sim800send("Hello from SIM800"))
        {
          display("Send OK");
          HAL_Delay(1000);
    } else {
          display("Send ERROR");
          HAL_Delay(1000);
        }
      } else {
        display("Connect ERROR");
        HAL_Delay(1000);
      }
      
   	  display("Disconnect");
      sim800disconnect();
      sendTelemetry = false;
    }
    
    /*
    // blink onboard blue LED
    HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
    if (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET) {
      // onboard key pressed
      HAL_Delay(500);
    }
    else
    {
      HAL_Delay(1000);
    }
    */
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_VBAT;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

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
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 4200;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1000;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

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
  huart1.Init.BaudRate = 57600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
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
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
  /* DMA2_Stream2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream2_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BAT_CHARGE_GPIO_Port, BAT_CHARGE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_BLUE_Pin */
  GPIO_InitStruct.Pin = LED_BLUE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_BLUE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : KEY_Pin */
  GPIO_InitStruct.Pin = KEY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(KEY_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BAT_CHARGE_Pin */
  GPIO_InitStruct.Pin = BAT_CHARGE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BAT_CHARGE_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
// every 50 ms - timer3
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
  static int cntADC = 0;
  static int cntKeyPress = 0;

  // blink onboard blue LED
  // HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);

  if (HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin) == GPIO_PIN_RESET) 
  {
    if(++cntKeyPress == 5) {
      // key pressed for 250 ms - display battery voltage
      displayBatteryVoltage();

      // displayScroll();
    } else if (cntKeyPress == 20)
    {
      // key pressed for 1 sec - switch ON onboard blue LED and send telemetry
      HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_RESET);
      sendTelemetry = true;
    }
  } else {
    cntKeyPress = 0;
    HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_SET);
  }
  
  if (++cntADC == 100)
  {
    cntADC = 0;
    // every 5 sec start ADC in DMA mode
    HAL_ADC_Start_DMA(&hadc1, adc_raw, 2);
  }  
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
  // stop ADC
  HAL_ADC_Stop_DMA(hadc);
  // Vin measured on A1 - 6k8/10k
  float vin = adc_raw[0] * 0.001358;
  // Vbat
  vbat = adc_raw[1] * 0.00339;

  if (vin > 4.5 && vbat < 4.0)
  {
    // charge battery
    HAL_GPIO_WritePin(BAT_CHARGE_GPIO_Port, BAT_CHARGE_Pin, GPIO_PIN_RESET);
  }
  else
  {
    HAL_GPIO_WritePin(BAT_CHARGE_GPIO_Port, BAT_CHARGE_Pin, GPIO_PIN_SET);
  }
 }

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(mavlink_parse_char(MAVLINK_COMM_0, uart1RX[0], &msg, &status))
	{
		if (msg.msgid == 0)
      return;

		if (msg.msgid == MAVLINK_MSG_ID_ATTITUDE) { // msgid == 30
			mavlink_attitude_t attitude;
			mavlink_msg_attitude_decode(&msg, &attitude);

			uint8_t text[80];
			sprintf((char *)&text, "yaw= %.2f\npitch= %.2f\nroll= %.2f", attitude.yaw, attitude.pitch, attitude.roll);
			// display((char *)&text);
      return;
		}

    if (msg.msgid == MAVLINK_MSG_ID_GLOBAL_POSITION_INT)
    {
      mavlink_global_position_int_t global_position_int;
      mavlink_msg_global_position_int_decode(&msg, &global_position_int);

			uint8_t text[80];
			sprintf((char *)&text, "lat= %ld\nlon= %ld\nalt= %ld", global_position_int.lat, global_position_int.lon, global_position_int.alt);

      return;
    }
	}
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
