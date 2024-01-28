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
#include "adc.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "motor.h"
#include "flash.h"
#include "helper.h"

#include "stm32f4xx_hal.h"

#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef enum {
  ADC_STATUS_INVALID,
  ADC_STATUS_STARTED,
  ADC_STATUS_FINISHED,
} ADC_status_E;

typedef enum {
  STATE_INIT,
  STATE_CALIBRATE_WHITE,
  STATE_CALIBRATE_BLACK,
  STATE_STOP,
  STATE_RUN,
  STATE_COUNT,
} state_E;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ADC_COUNT 6
#define ALPHA (0.9)
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static uint16_t adc_result[ADC_COUNT] = {0};
static volatile ADC_status_E adc_status = ADC_STATUS_INVALID;
static uint16_t adc_white[ADC_COUNT] = {65535};
static uint16_t adc_black[ADC_COUNT] = {0};

static state_E state = STATE_STOP;
static bool btn_dbc = false;

static double input_avg = 0.0f;

static void run_motor(void);
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
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  motor_init();
  motor_setFlip(M2, true);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    uint32_t tick = HAL_GetTick();
    char msg[11] = {0};
    bool btn = HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_SET;
    bool btn_rise = btn_dbc && !btn_dbc;

    switch(state) {
      case STATE_INIT:
        if(btn_rise) state = STATE_CALIBRATE_WHITE;
        msg[0] = '.';
        msg[1] = '\n';
        HAL_UART_Transmit(&huart2, (uint8_t*) msg, 2, 1000);
        state = STATE_CALIBRATE_WHITE;
        break;

      case STATE_CALIBRATE_WHITE:
        if(btn_rise) {
          switch(adc_status) {
            case ADC_STATUS_FINISHED:
            case ADC_STATUS_INVALID:
              HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_result, ADC_COUNT);
              adc_status = ADC_STATUS_STARTED;
              break;

            default:
              break;
          }
        }
        if(adc_status == ADC_STATUS_FINISHED) {
            for(size_t i = 0; i < ADC_COUNT; i++) {
              adc_white[i] = adc_result[i];
              size_t len = snprintf(msg, sizeof(msg), "% 10d", adc_result[i]);
              HAL_UART_Transmit(&huart2, (uint8_t*) msg, len, 1000);
            }
            msg[0] = '\n';
            HAL_UART_Transmit(&huart2, (uint8_t*) msg, 1, 1000);
            state = STATE_CALIBRATE_BLACK;
        }
        break;

      case STATE_CALIBRATE_BLACK:
        if(btn_rise) {
          switch(adc_status) {
            case ADC_STATUS_FINISHED:
            case ADC_STATUS_INVALID:
              HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_result, ADC_COUNT);
              adc_status = ADC_STATUS_STARTED;
              break;

            default:
              break;
          }
        }
        if(adc_status == ADC_STATUS_FINISHED) {
            for(size_t i = 0; i < ADC_COUNT; i++) {
              adc_black[i] = adc_result[i];
              size_t len = snprintf(msg, sizeof(msg), "% 10d", adc_result[i]);
              HAL_UART_Transmit(&huart2, (uint8_t*) msg, len, 1000);
            }
            msg[0] = '\n';
            HAL_UART_Transmit(&huart2, (uint8_t*) msg, 1, 1000);
            state = STATE_STOP;
        }
        break;

      case STATE_STOP:
        if(btn_rise) state = STATE_RUN;
        break;


      case STATE_RUN:
        switch(adc_status) {
          case ADC_STATUS_FINISHED:
          case ADC_STATUS_INVALID:
            HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc_result, ADC_COUNT);
            adc_status = ADC_STATUS_STARTED;
            break;

          default:
            break;
        }
        run_motor();
        break;

      default:
        break;
    }
  }
  /* USER CODE END 3 */
}

static void run_motor(void) {
    switch(adc_status) {
      case ADC_STATUS_FINISHED:
      {
        double new_input_avg = 0;
        for(size_t i = 0; i < ADC_COUNT; i++) {
          double normalized = NORMALIZE((double) adc_result[i], (double) adc_black[i], (double) adc_white[i]);
          new_input_avg += (i - 2.5) * SATURATE(normalized, 0, 1);
        }
        input_avg = input_avg * ALPHA + new_input_avg * (1 - ALPHA);
      }

      default:
        break;
    }

    char msg[15] = {0};
    size_t len = snprintf(msg, sizeof(msg), "%6.6f\n", input_avg);
    HAL_UART_Transmit(&huart2, (uint8_t*) msg, len, 1000);

    double Kp = 1000;

    double diff = input_avg * Kp;
    //int16_t speed = 20000;
    int16_t speed = 0;
    if(diff > 0) {
      motor_setSpeed(M1, SATURATE(speed - diff, 0, speed));
      motor_setSpeed(M2, speed);
    } else {
      motor_setSpeed(M1, speed);
      motor_setSpeed(M2, SATURATE(speed + diff, 0, speed));
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
  adc_status = ADC_STATUS_FINISHED;
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
     printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
