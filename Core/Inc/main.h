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
#include "stm32f4xx_hal.h"

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
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define MtrDvr_Fault_Pin GPIO_PIN_6
#define MtrDvr_Fault_GPIO_Port GPIOA
#define Servo_PWM_Pin GPIO_PIN_10
#define Servo_PWM_GPIO_Port GPIOB
#define MtrEnc_2A_Pin GPIO_PIN_6
#define MtrEnc_2A_GPIO_Port GPIOC
#define MtrEnc_2B_Pin GPIO_PIN_7
#define MtrEnc_2B_GPIO_Port GPIOC
#define MtrEnc_1A_Pin GPIO_PIN_8
#define MtrEnc_1A_GPIO_Port GPIOA
#define MtrEnc_1B_Pin GPIO_PIN_9
#define MtrEnc_1B_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define Servo_Jumper_Pin GPIO_PIN_12
#define Servo_Jumper_GPIO_Port GPIOC
#define MtrDvr_Dir1_Pin GPIO_PIN_3
#define MtrDvr_Dir1_GPIO_Port GPIOB
#define MtrDvr_Dir2_Pin GPIO_PIN_4
#define MtrDvr_Dir2_GPIO_Port GPIOB
#define MtrDvr_EN_Pin GPIO_PIN_5
#define MtrDvr_EN_GPIO_Port GPIOB
#define MtrDvr_PWM2_Pin GPIO_PIN_6
#define MtrDvr_PWM2_GPIO_Port GPIOB
#define MtrDvr_PWM1_Pin GPIO_PIN_8
#define MtrDvr_PWM1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
