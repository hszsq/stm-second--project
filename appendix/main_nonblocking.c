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
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
/* ============================================================
 * 三灯 500ms 流水灯 —— 非阻塞延时版（加分项）
 * LED0 = PA0, LED1 = PA1, LED2 = PA2（CubeMX User Label 生成）
 *
 * 原理：不使用 HAL_Delay() 死等。用 HAL_GetTick()（SysTick 1ms
 *       递增的时间戳）与上次切换时间 s_last_tick 比较，只有差值
 *       达到 500ms 才切换一次灯，其余时间主循环空转不阻塞。
 *       时间戳用"累加"而不是"赋值 now"，可消除累计误差。
 *
 * 说明：下面按"高电平点亮"写。如果你的 LED 是低电平点亮
 *       （蓝 Pill 板载灯那种），把 LED_ON / LED_OFF 里的
 *       GPIO_PIN_SET 和 GPIO_PIN_RESET 对调即可。
 * ============================================================ */
#define FLOW_INTERVAL_MS    500U

#define LED_ON(port, pin)   HAL_GPIO_WritePin((port), (pin), GPIO_PIN_SET)
#define LED_OFF(port, pin)  HAL_GPIO_WritePin((port), (pin), GPIO_PIN_RESET)

/* 非阻塞延时状态 */
static uint32_t s_last_tick = 0U;   /* 上次切换的时间戳(ms) */
static uint8_t  s_step      = 0U;   /* 当前点亮第几个灯 0~2 */

/* 只点亮 idx 号灯，其余熄灭 */
static void LED_ShowOnly(uint8_t idx)
{
  LED_OFF(LED0_GPIO_Port, LED0_Pin);
  LED_OFF(LED1_GPIO_Port, LED1_Pin);
  LED_OFF(LED2_GPIO_Port, LED2_Pin);

  switch (idx)
  {
    case 0U:  LED_ON(LED0_GPIO_Port, LED0_Pin); break;
    case 1U:  LED_ON(LED1_GPIO_Port, LED1_Pin); break;
    default:  LED_ON(LED2_GPIO_Port, LED2_Pin); break;
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
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    /* 记录首次进入的时间，避免 s_last_tick=0 导致第一拍立刻切灯 */
    if (s_last_tick == 0U)
    {
      s_last_tick = HAL_GetTick();
      LED_ShowOnly(s_step);
    }

    /* 非阻塞：每 500ms 才切换一次，其余时间主循环照常运行 */
    if ((HAL_GetTick() - s_last_tick) >= FLOW_INTERVAL_MS)
    {
      s_last_tick += FLOW_INTERVAL_MS;          /* 累加，消除累计误差 */
      s_step = (uint8_t)((s_step + 1U) % 3U);
      LED_ShowOnly(s_step);
    }
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
