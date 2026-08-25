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
#include <math.h>
#include "bmp3.h"
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

I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* =====================================================
   BMP390 I2C interface for Bosch BMP3 SensorAPI
   ===================================================== */

/*
 * Our BMP390 is using the secondary I2C address 0x77.
 */
static uint8_t bmp390_i2c_address = BMP3_ADDR_I2C_SEC;


/* =====================================================
   BMP390 I2C READ
   ===================================================== */

static BMP3_INTF_RET_TYPE bmp390_i2c_read(
    uint8_t reg_addr,
    uint8_t *read_data,
    uint32_t len,
    void *intf_ptr)
{
    uint8_t device_address =
        *((uint8_t *)intf_ptr);

    if (HAL_I2C_Mem_Read(
            &hi2c1,
            (device_address << 1),
            reg_addr,
            I2C_MEMADD_SIZE_8BIT,
            read_data,
            (uint16_t)len,
            100) == HAL_OK)
    {
        return BMP3_INTF_RET_SUCCESS;
    }

    return BMP3_E_COMM_FAIL;
}


/* =====================================================
   BMP390 I2C WRITE
   ===================================================== */

static BMP3_INTF_RET_TYPE bmp390_i2c_write(
    uint8_t reg_addr,
    const uint8_t *write_data,
    uint32_t len,
    void *intf_ptr)
{
    uint8_t device_address =
        *((uint8_t *)intf_ptr);

    if (HAL_I2C_Mem_Write(
            &hi2c1,
            (device_address << 1),
            reg_addr,
            I2C_MEMADD_SIZE_8BIT,
            (uint8_t *)write_data,
            (uint16_t)len,
            100) == HAL_OK)
    {
        return BMP3_INTF_RET_SUCCESS;
    }

    return BMP3_E_COMM_FAIL;
}


/* =====================================================
   BMP390 DELAY
   ===================================================== */

static void bmp390_delay_us(
    uint32_t period,
    void *intf_ptr)
{
    (void)intf_ptr;

    /*
     * HAL_Delay works in milliseconds.
     * Round microseconds upward so we never delay
     * for less time than Bosch requests.
     */
    uint32_t delay_ms =
        (period + 999U) / 1000U;

    if (delay_ms == 0U)
    {
        delay_ms = 1U;
    }

    HAL_Delay(delay_ms);
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
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  /* =========================
     Variables
     ========================= */

  /* Startup / sensor detection messages */
  uint8_t startup_message[] = "Avionics monitor started\r\n";

  uint8_t imu_found[] = "LSM6DSOX FOUND\r\n";
  uint8_t imu_missing[] = "LSM6DSOX NOT FOUND\r\n";

  uint8_t bmp_found[] = "BMP390 FOUND\r\n";
  uint8_t bmp_missing[] = "BMP390 NOT FOUND\r\n";


  /* =========================
     LSM6DSOX variables
     ========================= */

  /* WHO_AM_I register */
  uint8_t who_am_i = 0;
  char imu_id_message[64];

  /* Accelerometer */
  uint8_t accel_config = 0x40;
  uint8_t accel_data[6];

  int16_t accel_x;
  int16_t accel_y;
  int16_t accel_z;

  int32_t accel_x_mg = 0;
  int32_t accel_y_mg = 0;
  int32_t accel_z_mg = 0;

  /* Gyroscope */
  uint8_t gyro_config = 0x40;
  uint8_t gyro_data[6];

  int16_t gyro_x;
  int16_t gyro_y;
  int16_t gyro_z;

  int32_t gyro_x_mdps = 0;
  int32_t gyro_y_mdps = 0;
  int32_t gyro_z_mdps = 0;


  /* =========================
     BMP390 manual ID variables
     ========================= */

  uint8_t bmp_chip_id = 0;
  char bmp_id_message[64];


  /* =========================
     Bosch BMP3 driver variables
     ========================= */

  struct bmp3_dev bmp390_device = {0};

  struct bmp3_settings bmp390_settings = {0};

  struct bmp3_data bmp390_data = {0};

  uint32_t bmp390_settings_select = 0;

  int8_t bmp390_result = BMP3_OK;

  char bmp_driver_message[64];
  char bmp_config_message[64];


  /* =========================
     Battery ADC variables
     ========================= */

  uint32_t adc_raw = 0;
  uint32_t battery_mv = 0;


  /* Complete telemetry buffer */
  char telemetry_message[256];


  /* =========================
     Calibrate ADC
     ========================= */

  HAL_ADCEx_Calibration_Start(&hadc1);


  /* =========================
     Startup message
     ========================= */

  HAL_UART_Transmit(
      &huart2,
      startup_message,
      sizeof(startup_message) - 1U,
      HAL_MAX_DELAY
  );


  /* =========================
     Check LSM6DSOX
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
     Read LSM6DSOX WHO_AM_I
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


  /* =========================
     Check BMP390 manually
     ========================= */

  if (HAL_I2C_IsDeviceReady(
          &hi2c1,
          (0x77 << 1),
          3,
          100) == HAL_OK)
  {
      HAL_UART_Transmit(
          &huart2,
          bmp_found,
          sizeof(bmp_found) - 1U,
          HAL_MAX_DELAY
      );
  }
  else
  {
      HAL_UART_Transmit(
          &huart2,
          bmp_missing,
          sizeof(bmp_missing) - 1U,
          HAL_MAX_DELAY
      );
  }


  /* =========================
     Read BMP390 CHIP_ID manually
     ========================= */

  if (HAL_I2C_Mem_Read(
          &hi2c1,
          (0x77 << 1),
          0x00,
          I2C_MEMADD_SIZE_8BIT,
          &bmp_chip_id,
          1,
          100) == HAL_OK)
  {
      int bmp_id_length = snprintf(
          bmp_id_message,
          sizeof(bmp_id_message),
          "BMP390 CHIP_ID=0x%02X\r\n",
          bmp_chip_id
      );

      HAL_UART_Transmit(
          &huart2,
          (uint8_t *)bmp_id_message,
          (uint16_t)bmp_id_length,
          HAL_MAX_DELAY
      );
  }


  /* =========================
     Initialize Bosch BMP3 driver
     ========================= */

  bmp390_device.intf = BMP3_I2C_INTF;

  bmp390_device.read = bmp390_i2c_read;

  bmp390_device.write = bmp390_i2c_write;

  bmp390_device.delay_us = bmp390_delay_us;

  bmp390_device.intf_ptr = &bmp390_i2c_address;

  bmp390_result = bmp3_init(&bmp390_device);


  /* =========================
     Report Bosch driver status
     ========================= */

  if (bmp390_result == BMP3_OK)
  {
      int bmp_driver_length = snprintf(
          bmp_driver_message,
          sizeof(bmp_driver_message),
          "BMP390 DRIVER INIT OK CHIP_ID=0x%02X\r\n",
          bmp390_device.chip_id
      );

      HAL_UART_Transmit(
          &huart2,
          (uint8_t *)bmp_driver_message,
          (uint16_t)bmp_driver_length,
          HAL_MAX_DELAY
      );
  }
  else
  {
      int bmp_driver_length = snprintf(
          bmp_driver_message,
          sizeof(bmp_driver_message),
          "BMP390 DRIVER INIT FAILED=%d\r\n",
          bmp390_result
      );

      HAL_UART_Transmit(
          &huart2,
          (uint8_t *)bmp_driver_message,
          (uint16_t)bmp_driver_length,
          HAL_MAX_DELAY
      );
  }


  /* =========================
     Configure BMP390
     ========================= */

  if (bmp390_result == BMP3_OK)
  {
      /* Enable pressure and temperature sensing */
      bmp390_settings.press_en = BMP3_ENABLE;
      bmp390_settings.temp_en = BMP3_ENABLE;

      /*
       * Start with no oversampling for our first test.
       * We can increase precision later.
       */
      bmp390_settings.odr_filter.press_os =
          BMP3_NO_OVERSAMPLING;

      bmp390_settings.odr_filter.temp_os =
          BMP3_NO_OVERSAMPLING;

      /* Sensor produces new data at 25 Hz */
      bmp390_settings.odr_filter.odr =
          BMP3_ODR_25_HZ;


      /* Tell Bosch which settings we want changed */
      bmp390_settings_select =
          BMP3_SEL_PRESS_EN |
          BMP3_SEL_TEMP_EN |
          BMP3_SEL_PRESS_OS |
          BMP3_SEL_TEMP_OS |
          BMP3_SEL_ODR;


      bmp390_result = bmp3_set_sensor_settings(
          bmp390_settings_select,
          &bmp390_settings,
          &bmp390_device
      );


      /* Put BMP390 into continuous normal measurement mode */
      if (bmp390_result == BMP3_OK)
      {
          bmp390_settings.op_mode =
              BMP3_MODE_NORMAL;

          bmp390_result = bmp3_set_op_mode(
              &bmp390_settings,
              &bmp390_device
          );
      }


      /* Give the sensor time to begin producing measurements */
      HAL_Delay(50);
  }


  /* =========================
     Report BMP390 configuration
     ========================= */

  if (bmp390_result == BMP3_OK)
  {
      uint8_t bmp_config_ok[] =
          "BMP390 CONFIG OK\r\n";

      HAL_UART_Transmit(
          &huart2,
          bmp_config_ok,
          sizeof(bmp_config_ok) - 1U,
          HAL_MAX_DELAY
      );
  }
  else
  {
      int bmp_config_length = snprintf(
          bmp_config_message,
          sizeof(bmp_config_message),
          "BMP390 CONFIG FAILED=%d\r\n",
          bmp390_result
      );

      HAL_UART_Transmit(
          &huart2,
          (uint8_t *)bmp_config_message,
          (uint16_t)bmp_config_length,
          HAL_MAX_DELAY
      );
  }


  /* =========================
     Configure accelerometer
     ========================= */

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
	         Sensor health flags
	         ======================================== */

	      uint8_t imu_ok = 1U;
	      uint8_t bmp390_ok = 0U;


	      /* ========================================
	         Read accelerometer
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
	          accel_x = (int16_t)((accel_data[1] << 8) | accel_data[0]);
	          accel_y = (int16_t)((accel_data[3] << 8) | accel_data[2]);
	          accel_z = (int16_t)((accel_data[5] << 8) | accel_data[4]);

	          accel_x_mg = ((int32_t)accel_x * 61) / 1000;
	          accel_y_mg = ((int32_t)accel_y * 61) / 1000;
	          accel_z_mg = ((int32_t)accel_z * 61) / 1000;
	      }
	      else
	      {
	          /*
	           * Do not keep transmitting old accelerometer
	           * measurements if communication fails.
	           */
	          imu_ok = 0U;

	          accel_x_mg = 0;
	          accel_y_mg = 0;
	          accel_z_mg = 0;
	      }


	      /* ========================================
	         Read gyroscope
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
	          gyro_x = (int16_t)((gyro_data[1] << 8) | gyro_data[0]);
	          gyro_y = (int16_t)((gyro_data[3] << 8) | gyro_data[2]);
	          gyro_z = (int16_t)((gyro_data[5] << 8) | gyro_data[4]);

	          gyro_x_mdps = ((int32_t)gyro_x * 875) / 100;
	          gyro_y_mdps = ((int32_t)gyro_y * 875) / 100;
	          gyro_z_mdps = ((int32_t)gyro_z * 875) / 100;
	      }
	      else
	      {
	          /*
	           * If the gyro read fails, the IMU is
	           * considered unhealthy.
	           */
	          imu_ok = 0U;

	          gyro_x_mdps = 0;
	          gyro_y_mdps = 0;
	          gyro_z_mdps = 0;
	      }


	      /* ========================================
	         Read BMP390 temperature + pressure
	         ======================================== */

	      int32_t bmp_temp_centi_c = 0;
	      uint32_t bmp_pressure_pa = 0;
	      int32_t altitude_m_x100 = 0;

	      bmp390_result = bmp3_get_sensor_data(
	          BMP3_PRESS_TEMP,
	          &bmp390_data,
	          &bmp390_device
	      );

	      if (bmp390_result == BMP3_OK)
	      {
	          bmp390_ok = 1U;

	          /*
	           * Temperature:
	           * 24.15 C becomes 2415
	           */
	          bmp_temp_centi_c =
	              (int32_t)(bmp390_data.temperature * 100.0);

	          /*
	           * Pressure in Pascals
	           */
	          bmp_pressure_pa =
	              (uint32_t)(bmp390_data.pressure);


	          /* ========================================
	             Calculate barometric altitude
	             ======================================== */

	          if (bmp_pressure_pa > 0U)
	          {
	              double pressure_ratio =
	                  (double)bmp_pressure_pa / 101325.0;

	              double altitude_m =
	                  44330.0 *
	                  (1.0 - pow(pressure_ratio, 0.190294957));

	              altitude_m_x100 =
	                  (int32_t)(altitude_m * 100.0);
	          }
	      }
	      else
	      {
	          /*
	           * The BMP390 could not be read.
	           * Keep its outputs at zero rather than
	           * transmitting stale measurements.
	           */
	          bmp390_ok = 0U;

	          bmp_temp_centi_c = 0;
	          bmp_pressure_pa = 0;
	          altitude_m_x100 = 0;
	      }


	      /* ========================================
	         Read REAL battery voltage using ADC
	         ======================================== */

	      HAL_ADC_Start(&hadc1);

	      if (HAL_ADC_PollForConversion(
	              &hadc1,
	              100) == HAL_OK)
	      {
	          adc_raw = HAL_ADC_GetValue(&hadc1);

	          battery_mv =
	              ((uint64_t)adc_raw * 3300U * 133U) /
	              (4095U * 33U);
	      }

	      HAL_ADC_Stop(&hadc1);


	      /* ========================================
	         Determine complete system status
	         ======================================== */

	      uint8_t low_battery =
	          (battery_mv < 7000U);

	      const char *system_status;

	      if (low_battery && !imu_ok && !bmp390_ok)
	      {
	          system_status = "LOW_BATT_IMU_BMP_FAULT";
	      }
	      else if (low_battery && !imu_ok)
	      {
	          system_status = "LOW_BATT_IMU_FAULT";
	      }
	      else if (low_battery && !bmp390_ok)
	      {
	          system_status = "LOW_BATT_BMP_FAULT";
	      }
	      else if (!imu_ok && !bmp390_ok)
	      {
	          system_status = "IMU_BMP_FAULT";
	      }
	      else if (!imu_ok)
	      {
	          system_status = "IMU_FAULT";
	      }
	      else if (!bmp390_ok)
	      {
	          system_status = "BMP_FAULT";
	      }
	      else if (low_battery)
	      {
	          system_status = "LOW_BATTERY";
	      }
	      else
	      {
	          system_status = "OK";
	      }


	      /* ========================================
	         Create complete telemetry packet
	         ======================================== */

	      int message_length = snprintf(
	          telemetry_message,
	          sizeof(telemetry_message),
	          "TIME_MS=%lu,"
	          "BATTERY_MV=%lu,"
	          "STATUS=%s,"
	          "ACCEL_X_MG=%ld,"
	          "ACCEL_Y_MG=%ld,"
	          "ACCEL_Z_MG=%ld,"
	          "GYRO_X_MDPS=%ld,"
	          "GYRO_Y_MDPS=%ld,"
	          "GYRO_Z_MDPS=%ld,"
	          "TEMP_C_X100=%ld,"
	          "PRESSURE_PA=%lu,"
	          "ALTITUDE_M_X100=%ld\r\n",
	          (unsigned long)HAL_GetTick(),
	          (unsigned long)battery_mv,
	          system_status,
	          (long)accel_x_mg,
	          (long)accel_y_mg,
	          (long)accel_z_mg,
	          (long)gyro_x_mdps,
	          (long)gyro_y_mdps,
	          (long)gyro_z_mdps,
	          (long)bmp_temp_centi_c,
	          (unsigned long)bmp_pressure_pa,
	          (long)altitude_m_x100
	      );


	      /* ========================================
	         Send telemetry over UART
	         ======================================== */

	      HAL_UART_Transmit(
	          &huart2,
	          (uint8_t *)telemetry_message,
	          (uint16_t)message_length,
	          HAL_MAX_DELAY
	      );


	      /* One complete telemetry packet per second */
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
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.LowPowerAutoPowerOff = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.SamplingTimeCommon1 = ADC_SAMPLETIME_79CYCLES_5;
  hadc1.Init.SamplingTimeCommon2 = ADC_SAMPLETIME_1CYCLE_5;
  hadc1.Init.OversamplingMode = DISABLE;
  hadc1.Init.TriggerFrequencyMode = ADC_TRIGGER_FREQ_HIGH;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
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
