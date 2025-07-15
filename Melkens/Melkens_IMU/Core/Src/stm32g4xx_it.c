/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32g4xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32g4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "IMU_func.h"
#include "UartHandler.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */
//TIM_HandleTypeDef htim1;
//TIM_HandleTypeDef htim16;

/* USER CODE END TD */

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
/* USER CODE BEGIN PFP */
void TimeManager_SYSTICK_Handler();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */
  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */
//	HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32G4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32g4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles DMA1 channel1 global interrupt.
  */
void DMA1_Channel1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel1_IRQn 0 */
if(LL_DMA_IsActiveFlag_TC1(DMA1))
			{
				LL_DMA_ClearFlag_TC1(DMA1);
				LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_1);
				UartHandler_SetMessageReceived(Uart_ConnectivityESP);
			}
			if(LL_DMA_IsActiveFlag_HT1(DMA1))
			{
				LL_DMA_ClearFlag_HT1(DMA1);
			}
			if(LL_DMA_IsActiveFlag_TE1(DMA1))
			{
				LL_DMA_ClearFlag_TE1(DMA1);
			}
  /* USER CODE END DMA1_Channel1_IRQn 0 */

  /* USER CODE BEGIN DMA1_Channel1_IRQn 1 */

  /* USER CODE END DMA1_Channel1_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel2 global interrupt.
  */
void DMA1_Channel2_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel2_IRQn 0 */
	if(LL_DMA_IsActiveFlag_TC2(DMA1))
{
			LL_DMA_ClearFlag_TC2(DMA1);
			LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
		}
		if(LL_DMA_IsActiveFlag_HT2(DMA1))
		{
			LL_DMA_ClearFlag_HT2(DMA1);
		}
		if(LL_DMA_IsActiveFlag_TE2(DMA1))
		{
			LL_DMA_ClearFlag_TE2(DMA1);
		}
  /* USER CODE END DMA1_Channel2_IRQn 0 */

  /* USER CODE BEGIN DMA1_Channel2_IRQn 1 */

  /* USER CODE END DMA1_Channel2_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel3 global interrupt.
  */
void DMA1_Channel3_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel3_IRQn 0 */
	if(LL_DMA_IsActiveFlag_TC3(DMA1))
	{
		LL_DMA_ClearFlag_TC3(DMA1);
		LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);
	}
	if(LL_DMA_IsActiveFlag_HT3(DMA1))
	{
		LL_DMA_ClearFlag_HT3(DMA1);
	}
	if(LL_DMA_IsActiveFlag_TE3(DMA1))
	{
		LL_DMA_ClearFlag_TE3(DMA1);
	}
  /* USER CODE END DMA1_Channel3_IRQn 0 */

  /* USER CODE BEGIN DMA1_Channel3_IRQn 1 */

  /* USER CODE END DMA1_Channel3_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel4 global interrupt.
  */
void DMA1_Channel4_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel4_IRQn 0 */
	if(LL_DMA_IsActiveFlag_TC4(DMA1))
	{
		LL_DMA_ClearFlag_TC4(DMA1);
		/*
		 * TBD - Enable overrun iterrupt and dont disable channel to avoid
		 * loosing frames!
		 * */
		LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4);
		UartHandler_SetMessageReceived(Uart_3);
	}
	if(LL_DMA_IsActiveFlag_HT4(DMA1))
	{
		LL_DMA_ClearFlag_HT4(DMA1);
	}
	if(LL_DMA_IsActiveFlag_TE4(DMA1))
	{
		LL_DMA_ClearFlag_TE4(DMA1);
	}
  /* USER CODE END DMA1_Channel4_IRQn 0 */

  /* USER CODE BEGIN DMA1_Channel4_IRQn 1 */

  /* USER CODE END DMA1_Channel4_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[15:10] interrupts.
  */
void EXTI15_10_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI15_10_IRQn 0 */

  /* USER CODE END EXTI15_10_IRQn 0 */
  if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_10) != RESET)
  {
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_10);
    /* USER CODE BEGIN LL_EXTI_LINE_10 */
    IMU_SetDataReadyFlag();
    /* USER CODE END LL_EXTI_LINE_10 */
  }
  if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_11) != RESET)
  {
    LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_11);
    /* USER CODE BEGIN LL_EXTI_LINE_11 */

    /* USER CODE END LL_EXTI_LINE_11 */
  }
  /* USER CODE BEGIN EXTI15_10_IRQn 1 */

  /* USER CODE END EXTI15_10_IRQn 1 */
}

/**
  * @brief This function handles TIM7 global interrupt, DAC2 and DAC4 channel underrun error interrupts.
  */
void TIM7_DAC_IRQHandler(void)
{
  /* USER CODE BEGIN TIM7_DAC_IRQn 0 */
	 if(LL_TIM_IsActiveFlag_UPDATE(TIM7))
		  {
			  //HAL_GPIO_WritePin(DEGUB_2_GPIO_Port, DEGUB_2_Pin, GPIO_PIN_SET);

			  LL_TIM_ClearFlag_UPDATE(TIM7);
			  TimeManager_SYSTICK_Handler();

			  //HAL_GPIO_WritePin(DEGUB_2_GPIO_Port, DEGUB_2_Pin, GPIO_PIN_RESET);


		  }


  /* USER CODE END TIM7_DAC_IRQn 0 */

  /* USER CODE BEGIN TIM7_DAC_IRQn 1 */

  /* USER CODE END TIM7_DAC_IRQn 1 */
}

/**
  * @brief This function handles DMA2 channel1 global interrupt.
  */
void DMA2_Channel1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Channel1_IRQn 0 */

	if(LL_DMA_IsActiveFlag_TC1(DMA2))
	{
		LL_DMA_ClearFlag_TC1(DMA2);
		UartHandler_SetMessageReceived(Uart_PMB);//message recieved flag
		LL_USART_Disable(USART2);
		//LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_1);
	}
	if(LL_DMA_IsActiveFlag_HT1(DMA2))
	{
		LL_DMA_ClearFlag_HT1(DMA2);
	}
	if(LL_DMA_IsActiveFlag_TE1(DMA2))
	{
		LL_DMA_ClearFlag_TE1(DMA2);
	}

  /* USER CODE END DMA2_Channel1_IRQn 0 */

  /* USER CODE BEGIN DMA2_Channel1_IRQn 1 */

  /* USER CODE END DMA2_Channel1_IRQn 1 */
}

/**
  * @brief This function handles DMA2 channel2 global interrupt.
  */
void DMA2_Channel2_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Channel2_IRQn 0 */
	if(LL_DMA_IsActiveFlag_TC2(DMA2))
	{
		LL_DMA_ClearFlag_TC2(DMA2);

		LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_2);
	}
	if(LL_DMA_IsActiveFlag_HT2(DMA2))
	{
		LL_DMA_ClearFlag_HT2(DMA2);
	}
	if(LL_DMA_IsActiveFlag_TE2(DMA2))
	{
		LL_DMA_ClearFlag_TE2(DMA2);
	}
  /* USER CODE END DMA2_Channel2_IRQn 0 */

  /* USER CODE BEGIN DMA2_Channel2_IRQn 1 */

  /* USER CODE END DMA2_Channel2_IRQn 1 */
}

/**
  * @brief This function handles DMA2 channel3 global interrupt.
  */
void DMA2_Channel3_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Channel3_IRQn 0 */
	if(LL_DMA_IsActiveFlag_TC3(DMA2))
	{
		LL_DMA_ClearFlag_TC3(DMA2);
		LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_3);
		UartHandler_SetMessageReceived(Uart_5);
	}
	if(LL_DMA_IsActiveFlag_HT3(DMA2))
	{
		LL_DMA_ClearFlag_HT3(DMA2);
	}
	if(LL_DMA_IsActiveFlag_TE3(DMA2))
	{
		LL_DMA_ClearFlag_TE3(DMA2);
	}
  /* USER CODE END DMA2_Channel3_IRQn 0 */

  /* USER CODE BEGIN DMA2_Channel3_IRQn 1 */

  /* USER CODE END DMA2_Channel3_IRQn 1 */
}

/**
  * @brief This function handles DMA2 channel4 global interrupt.
  */
void DMA2_Channel4_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Channel4_IRQn 0 */
	if(LL_DMA_IsActiveFlag_TC4(DMA2))
	{
		LL_DMA_ClearFlag_TC4(DMA2);
		LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_4);
	}
	if(LL_DMA_IsActiveFlag_HT4(DMA2))
	{
		LL_DMA_ClearFlag_HT4(DMA2);
	}
	if(LL_DMA_IsActiveFlag_TE4(DMA2))
	{
		LL_DMA_ClearFlag_TE4(DMA2);
	}
  /* USER CODE END DMA2_Channel4_IRQn 0 */

  /* USER CODE BEGIN DMA2_Channel4_IRQn 1 */

  /* USER CODE END DMA2_Channel4_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
