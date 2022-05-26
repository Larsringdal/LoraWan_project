/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "adc.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lora.h"
#include <stdio.h>
#include <string.h>

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

/* USER CODE BEGIN PV */
uint8_t c[2] = {0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
uint16_t readSensor();

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  MX_USART6_UART_Init();
  /* USER CODE BEGIN 2 */

     // RESET LORA
	HAL_GPIO_WritePin(LORA_RST_GPIO_Port, LORA_RST_Pin, 1);
	HAL_Delay(500);
	HAL_GPIO_WritePin(LORA_RST_GPIO_Port, LORA_RST_Pin, 0);
	HAL_Delay(500);
	HAL_GPIO_WritePin(LORA_RST_GPIO_Port, LORA_RST_Pin, 1);
	HAL_Delay(500);

	// Clear UART buffer before main code execution
	HAL_StatusTypeDef ret = HAL_OK;
	while (ret != HAL_TIMEOUT) ret = HAL_UART_Receive(&huart6, c, 1, 100);
	// INIT LORA
	lora_instance lora;
	init_lora_instance(&huart6, &lora);
	// PRINT HWEUI
	HAL_UART_Transmit(&huart2, (uint8_t*)"Started LoRa: ", 14, 100);
	HAL_UART_Transmit(&huart2, lora.hweui, strlen((char*)lora.hweui), 100);
	HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n\r\n", 2, 100);

	HAL_Delay(200);

	lora_response lo = lora_joinOtaa(&lora);
	if (lo){
		uint8_t bu[50] = {0};
		sprintf((char*)bu, "Error joining OTAA with code: %i\r\n", lo);
		HAL_UART_Transmit(&huart2, bu, strlen((char*)bu), 100);
	}else{
		HAL_UART_Transmit(&huart2, (uint8_t*)"Joined OTAA\r\n", 13, 100);
	}
  /* USER CODE END 2 */

	uint16_t dustval;
	uint8_t tt[100] = {0};
	dustval = readSensor();
	sprintf((char*)tt, "dust val = %i\r\n", dustval);
	HAL_UART_Transmit(&huart2, tt, strlen((char*)tt), 100);
	lora_sendData(&lora, dustval);

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
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

/* USER CODE BEGIN 4 */
uint16_t readSensor(){

	uint16_t DustValue = 0;
	/* Activates the Infrared light */
	HAL_GPIO_WritePin(IR_ctrl_GPIO_Port, IR_ctrl_Pin, 0);
	/* Wait to ensure active LED*/
	HAL_Delay(280);
	/* Activate ADC */
	HAL_ADC_Start(&hadc1);
	/* Get current Dust Voltage */
	HAL_ADC_PollForConversion(&hadc1, 100);
	DustValue = HAL_ADC_GetValue(&hadc1);
	/* Wait to ensure Data has been correctly read */
	HAL_Delay(40);
	HAL_GPIO_WritePin(IR_ctrl_GPIO_Port, IR_ctrl_Pin, 1);
	/* Wait remaining 99% of Duty-Cycle*/
	HAL_Delay(1000);

	return DustValue;
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

