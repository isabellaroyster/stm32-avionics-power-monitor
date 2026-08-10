/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include <stdio.h>
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
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

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
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  /* =========================
     Variables
     ========================= */

  /* Startup / sensor detection messages */
  uint8_t startup_message[] = "Avionics monitor started\r\n";
  uint8_t imu_found[] = "LSM6DSOX FOUND\r\n";
  uint8_t imu_missing[] = "LSM6DSOX NOT FOUND\r\n";

  /* WHO_AM_I register data */
  uint8_t who_am_i = 0;
  char imu_id_message[64];

  /* Accelerometer configuration and data */
  uint8_t accel_config = 0x40;
  uint8_t accel_data[6];

  int16_t accel_x;
  int16_t accel_y;
  int16_t accel_z;

  /* Gyroscope configuration and data */
  uint8_t gyro_config = 0x40;
  uint8_t gyro_data[6];

  int16_t gyro_x;
  int16_t gyro_y;
  int16_t gyro_z;

  /* Battery telemetry */
  char telemetry_message[96];
  uint32_t simulated_battery_mv = 8400U;


  /* =========================
     Startup UART message
     ========================= */

  HAL_UART_Transmit(
      &huart2,
      startup_message,
      sizeof(startup_message) - 1U,
      HAL_MAX_DELAY
  );


  /* =========================
     Check for LSM6DSOX
     ========================= */

  if (HAL_I2C_IsDeviceReady(
          &hi2c1,
          (0x6A << 1),
          3,
          100) == HAL_OK)
  {
      HAL_UART_Transmit(
          &huart2,
          imu_found,
          sizeof(imu_found) - 1U,
          HAL_MAX_DELAY
      );
  }
  else
  {
      HAL_UART_Transmit(
          &huart2,
          imu_missing,
          sizeof(imu_missing) - 1U,
          HAL_MAX_DELAY
      );
  }


  /* =========================
     Read WHO_AM_I register
     ========================= */

  if (HAL_I2C_Mem_Read(
          &hi2c1,
          (0x6A << 1),
          0x0F,
          I2C_MEMADD_SIZE_8BIT,
          &who_am_i,
          1,
          100) == HAL_OK)
  {
      int id_message_length = snprintf(
          imu_id_message,
          sizeof(imu_id_message),
          "LSM6DSOX WHO_AM_I=0x%02X\r\n",
          who_am_i
      );

      HAL_UART_Transmit(
          &huart2,
          (uint8_t *)imu_id_message,
          (uint16_t)id_message_length,
          HAL_MAX_DELAY
      );
  }
  else
  {
      uint8_t read_error[] = "WHO_AM_I READ FAILED\r\n";

      HAL_UART_Transmit(
          &huart2,
          read_error,
          sizeof(read_error) - 1U,
          HAL_MAX_DELAY
      );
  }


  /* =========================
     Configure accelerometer
     ========================= */

  /*
   * CTRL1_XL register = 0x10
   * 0x40 = 104 Hz output data rate
   *        +/- 2 g measurement range
   */
  HAL_I2C_Mem_Write(
      &hi2c1,
      (0x6A << 1),
      0x10,
      I2C_MEMADD_SIZE_8BIT,
      &accel_config,
      1,
      100
  );


  /* =========================
     Configure gyroscope
     ========================= */

  /*
   * CTRL2_G register = 0x11
   * 0x40 = 104 Hz output data rate
   *        +/- 250 degrees/second range
   */
  HAL_I2C_Mem_Write(
      &hi2c1,
      (0x6A << 1),
      0x11,
      I2C_MEMADD_SIZE_8BIT,
      &gyro_config,
      1,
      100
  );

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	    /* ========================================
	       Read accelerometer data
	       ======================================== */

	    if (HAL_I2C_Mem_Read(
	            &hi2c1,
	            (0x6A << 1),
	            0x28,
	            I2C_MEMADD_SIZE_8BIT,
	            accel_data,
	            6,
	            100) == HAL_OK)
	    {
	        /* Combine low + high bytes into 16-bit values */
	        accel_x = (int16_t)((accel_data[1] << 8) | accel_data[0]);
	        accel_y = (int16_t)((accel_data[3] << 8) | accel_data[2]);
	        accel_z = (int16_t)((accel_data[5] << 8) | accel_data[4]);

	        /* Convert raw counts to milli-g */
	        int32_t accel_x_mg = ((int32_t)accel_x * 61) / 1000;
	        int32_t accel_y_mg = ((int32_t)accel_y * 61) / 1000;
	        int32_t accel_z_mg = ((int32_t)accel_z * 61) / 1000;

	        char accel_message[96];

	        int accel_length = snprintf(
	            accel_message,
	            sizeof(accel_message),
	            "ACCEL_MG X=%ld Y=%ld Z=%ld\r\n",
	            (long)accel_x_mg,
	            (long)accel_y_mg,
	            (long)accel_z_mg
	        );

	        HAL_UART_Transmit(
	            &huart2,
	            (uint8_t *)accel_message,
	            (uint16_t)accel_length,
	            HAL_MAX_DELAY
	        );
	    }


	    /* ========================================
	       Read gyroscope data
	       ======================================== */

	    if (HAL_I2C_Mem_Read(
	            &hi2c1,
	            (0x6A << 1),
	            0x22,
	            I2C_MEMADD_SIZE_8BIT,
	            gyro_data,
	            6,
	            100) == HAL_OK)
	    {
	        /* Combine low + high bytes into 16-bit values */
	        gyro_x = (int16_t)((gyro_data[1] << 8) | gyro_data[0]);
	        gyro_y = (int16_t)((gyro_data[3] << 8) | gyro_data[2]);
	        gyro_z = (int16_t)((gyro_data[5] << 8) | gyro_data[4]);

	        /* Convert raw values to milli-degrees per second */
	        int32_t gyro_x_mdps = ((int32_t)gyro_x * 875) / 100;
	        int32_t gyro_y_mdps = ((int32_t)gyro_y * 875) / 100;
	        int32_t gyro_z_mdps = ((int32_t)gyro_z * 875) / 100;

	        char gyro_message[96];

	        int gyro_length = snprintf(
	            gyro_message,
	            sizeof(gyro_message),
	            "GYRO_MDPS X=%ld Y=%ld Z=%ld\r\n",
	            (long)gyro_x_mdps,
	            (long)gyro_y_mdps,
	            (long)gyro_z_mdps
	        );

	        HAL_UART_Transmit(
	            &huart2,
	            (uint8_t *)gyro_message,
	            (uint16_t)gyro_length,
	            HAL_MAX_DELAY
	        );
	    }


	    /* ========================================
	       Determine battery status
	       ======================================== */

	    const char *system_status;

	    if (simulated_battery_mv < 7000U)
	    {
	        system_status = "LOW_BATTERY";
	    }
	    else
	    {
	        system_status = "OK";
	    }


	    /* ========================================
	       Create battery telemetry
	       ======================================== */

	    int message_length = snprintf(
	        telemetry_message,
	        sizeof(telemetry_message),
	        "TIME_MS=%lu,BATTERY_MV=%lu,STATUS=%s\r\n",
	        (unsigned long)HAL_GetTick(),
	        (unsigned long)simulated_battery_mv,
	        system_status
	    );


	    /* Send battery telemetry */
	    HAL_UART_Transmit(
	        &huart2,
	        (uint8_t *)telemetry_message,
	        (uint16_t)message_length,
	        HAL_MAX_DELAY
	    );


	    /* ========================================
	       Simulate battery discharge
	       ======================================== */

	    if (simulated_battery_mv > 6000U)
	    {
	        simulated_battery_mv -= 200U;
	    }
	    else
	    {
	        simulated_battery_mv = 8400U;
	    }


	    /* Wait approximately one second */
	    HAL_Delay(1000);
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
  hi2c1.Init.Timing = 0x00503D58;
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

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_GREEN_Pin */
  GPIO_InitStruct.Pin = LED_GREEN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(LED_GREEN_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

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
#ifdef USE_FULL_ASSERT
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
