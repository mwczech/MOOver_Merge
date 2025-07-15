/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"
#include "stm32g4xx_ll_lpuart.h"
#include "stm32g4xx_ll_rcc.h"
#include "stm32g4xx_ll_tim.h"
#include "stm32g4xx_ll_usart.h"
#include "stm32g4xx_ll_system.h"
#include "stm32g4xx_ll_gpio.h"
#include "stm32g4xx_ll_exti.h"
#include "stm32g4xx_ll_bus.h"
#include "stm32g4xx_ll_cortex.h"
#include "stm32g4xx_ll_utils.h"
#include "stm32g4xx_ll_pwr.h"
#include "stm32g4xx_ll_dma.h"

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
#define CAN_Transciever_silence_Pin LL_GPIO_PIN_13
#define CAN_Transciever_silence_GPIO_Port GPIOC
#define CAN_Transciever_silenceC14_Pin LL_GPIO_PIN_14
#define CAN_Transciever_silenceC14_GPIO_Port GPIOC
#define CAN_Transciever_silenceC15_Pin LL_GPIO_PIN_15
#define CAN_Transciever_silenceC15_GPIO_Port GPIOC
#define ESP_comunication_RX_Pin LL_GPIO_PIN_0
#define ESP_comunication_RX_GPIO_Port GPIOC
#define ESP_comunication_TX_Pin LL_GPIO_PIN_1
#define ESP_comunication_TX_GPIO_Port GPIOC
#define ESP_enable_Pin LL_GPIO_PIN_2
#define ESP_enable_GPIO_Port GPIOA
#define ext_flash_hold_Pin LL_GPIO_PIN_3
#define ext_flash_hold_GPIO_Port GPIOA
#define ext_flash_cs_Pin LL_GPIO_PIN_4
#define ext_flash_cs_GPIO_Port GPIOA
#define ext_flash_SCK_Pin LL_GPIO_PIN_5
#define ext_flash_SCK_GPIO_Port GPIOA
#define ext_flash_MISO_Pin LL_GPIO_PIN_6
#define ext_flash_MISO_GPIO_Port GPIOA
#define ext_flash_MOSI_Pin LL_GPIO_PIN_7
#define ext_flash_MOSI_GPIO_Port GPIOA
#define ext_flash_write_protect_Pin LL_GPIO_PIN_4
#define ext_flash_write_protect_GPIO_Port GPIOC
#define LED1_Pin LL_GPIO_PIN_0
#define LED1_GPIO_Port GPIOB
#define LED2_Pin LL_GPIO_PIN_1
#define LED2_GPIO_Port GPIOB
#define LED3_Pin LL_GPIO_PIN_2
#define LED3_GPIO_Port GPIOB
#define optional_magnetometer_SCL_Pin LL_GPIO_PIN_6
#define optional_magnetometer_SCL_GPIO_Port GPIOC
#define optional_magnetometer_SDA_Pin LL_GPIO_PIN_7
#define optional_magnetometer_SDA_GPIO_Port GPIOC
#define optional_magnetometer_data_ready_Pin LL_GPIO_PIN_8
#define optional_magnetometer_data_ready_GPIO_Port GPIOC
#define optional_magnetometer_interrupt_Pin LL_GPIO_PIN_9
#define optional_magnetometer_interrupt_GPIO_Port GPIOC
#define lsm6dsr_SDA_Pin LL_GPIO_PIN_8
#define lsm6dsr_SDA_GPIO_Port GPIOA
#define lsm6dsr_SCL_Pin LL_GPIO_PIN_9
#define lsm6dsr_SCL_GPIO_Port GPIOA
#define sensor_bar_on_of_Pin LL_GPIO_PIN_15
#define sensor_bar_on_of_GPIO_Port GPIOA
#define lsm6dsr_int2_Pin LL_GPIO_PIN_10
#define lsm6dsr_int2_GPIO_Port GPIOC
#define lsm6dsr_int2_EXTI_IRQn EXTI15_10_IRQn
#define lsm6dsr_int1_Pin LL_GPIO_PIN_11
#define lsm6dsr_int1_GPIO_Port GPIOC
#define lsm6dsr_int1_EXTI_IRQn EXTI15_10_IRQn

/* USER CODE BEGIN Private defines */

//static void tx_com( uint8_t *tx_buffer, uint16_t len );
//static void platform_delay(uint32_t ms);
//static void platform_init(void);

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
