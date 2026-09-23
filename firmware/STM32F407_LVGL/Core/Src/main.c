/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "tim.h"
#include "uart1.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "app_config.h"

#include "LCD_Init.h"
#include "LCD_Disp.h"
#include "LCD_Touch.h"

#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"
#include "freertos_test.h"
#include "sys.h"
#include "led.h"
#include "beep.h"
#include "keyboard.h"
#include "lock.h"
#include "oled.h"
#include "password.h"
#include "w25q128.h"
#include "curtain.h"
#include "bh1750.h"
#include "bluetooth.h"
#include "voice.h"
#include "l610.h"
#include "onenet.h"
#include "weather.h"
#include "My_GUI.h"

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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

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
  HAL_Init();
  SystemClock_Config();
  delay_init();

  MX_GPIO_Init();
  uart1_init(115200);
  uart3_init(115200);
  MX_TIM6_Init();
  HAL_TIM_Base_Start_IT(&htim6);

  APP_LOG("=====================================\r\n");
  APP_LOG("       STM32F407ZGT6 系统启动        \r\n");
  APP_LOG("=====================================\r\n");

  /* 上电防抖：等电源/外设稳定；过长会拖慢演示，可按硬件缩短 */
  delay_ms(1200);

  beep_init();
  keyboard_init();
  lock_init();
  oled_init();
  led_init();
  bh1750_init();
  password_init();
  w25q128_init();

  /* L610：蜂窝注册 → TCP 心知(HTTP) → 取 3 天预报 → 断开 → TCP OneNET 透传(MQTT) */
  if (l610_modem_init_link() != L610_EOK)
  {
      APP_LOG("L610: modem init fail\r\n");
  }
  else
  {
      /* 蜂窝注册后、进透传前：AT+CCLK? 只取时:分:秒 → My_GUI 时间面板初值 */
      {
          uint8_t ch, cm, cs;
          if (l610_modem_query_cclk_hms(&ch, &cm, &cs) == L610_EOK)
          {
              my_gui_set_clock_hms(ch, cm, cs);
              APP_LOG("L610: CCLK hms %02u:%02u:%02u\r\n", (unsigned)ch, (unsigned)cm, (unsigned)cs);
          }
          else
          {
              APP_LOG("L610: CCLK? parse fail (keep 0:0:0)\r\n");
          }
      }

      if (l610_tcp_transparent_open(L610_WEATHER_SERVER_IP, L610_WEATHER_SERVER_PORT) == L610_EOK)
      {
          get_today_weather();
          l610_tcp_transparent_close();
      }
      else
      {
          APP_LOG("L610: weather TCP skip\r\n");
      }

      if (l610_tcp_transparent_open(L610_TCP_SERVER_IP, L610_TCP_SERVER_PORT) != L610_EOK)
      {
          APP_LOG("L610: OneNET TCP open fail\r\n");
      }
  }

  // MQTT 初始化
  mqtt_init();
  for(int i = 0; i < 5; i++)
  {
      APP_LOG("MQTT 第 %d 次连接...\r\n", i+1);
      uint8_t result = mqtt_connect(MQTT_ClientID, MQTT_UserName, MQTT_PassWord);
      if(result != 0)
      {
          APP_LOG("MQTT 第 %d 次连接失败...\r\n", i+1);
          delay_ms(1000);
      }
      else
      {
          APP_LOG("MQTT 第 %d 次连接成功！\r\n", i+1);
          (void)mqtt_subscribe_topic(SET_TOPIC, 0, 1);
          delay_ms(300);
          break;
      }
  }

  curtain_init();
  bt_init(9600);
  voice_init();

  APP_LOG("hello world!\r\n");
  password_check();
  led_living_off();

  freertos_test();
  vTaskStartScheduler();

  for (;;) {

  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 288;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

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

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */