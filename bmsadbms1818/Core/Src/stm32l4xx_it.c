/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32l4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32l4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "morse.h"
#include "bmsspi.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

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

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_i2c2_rx;
extern DMA_HandleTypeDef hdma_i2c2_tx;
extern DMA_HandleTypeDef hdma_spi1_rx;
extern DMA_HandleTypeDef hdma_spi1_tx;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim15;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef htim16;

/* USER CODE BEGIN EV */
/* Registers from Hard Fault */

/* Saved registers -- in the following order:
0 - psr was stacked by Hard Fault; 4 - 10 remain.
  0, 1, 2, 3, 12, lr, pc, psr, 4, 5, 6, 7, 8, 9, 10 */
volatile uint32_t reg_stack[16];


volatile uint32_t sys_regs[5];
/* sys_regs: Four System registers saved. 0xE000ED30 skipped.
hence, 5 words reserved.

[0] 0xE000ED28 
  Memory Management Fault Status Register (byte)
   7 MMAR is valid
   4 Stacking error
   3 Unstacking error
   1 Data access violation
   0 Instruction access violation

  Bus Fault Status Register (byte)
   7 BFAR is valid
   4 Stacking error
   3 Unstacking error
   2 Imprecise data access violation
   1 Precise data access violation
   0 Instruction access violation
 
  Usage Fault Status Register (half word)
   9 Divide by zero (only set if DIV_)_TRP set)
   8 Unaligned access 
   3 Attempt to execut coprocessor instruction
   2 Attempt to do exception with bad value in EXC_RETURN
   1 Attempt to switch to invalid state (e.g. ARM)
   0 Attempt to execute an undefined instruction

[1] 0xE000ED2C Hard Fault Status Register (word)
 31 Triggered by debugg event
 30 Taken because bus fault/mem mngment fault/usage fault
  1 Vector fetch

[2] 0xE000ED30 Debug Fault Status Register (word)
 not saved

[3] 0xE000ED34 Memory Manage Address Register (word)
 Address that caused memory manage fault

[4] 0xE000ED38 Bus Fault Manage Address Register (word)
 Address that caused the bus fault
*/
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
     morse_trap(000);
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */
/* The prototype shows it is a naked function - in effect this is just an
assembly function. */
void HardFault_Handler( void ) __attribute__( ( naked ) );

/* The fault handler implementation calls a function called
prvGetRegistersFromStack(). */
__asm volatile
 (
   " tst lr, #4                       \n\t" /* Determine stack in use. */ 
   " ite eq                           \n\t" /* Set R0 with stack ptr */
   " mrseq r0, msp                    \n\t" 
   " mrsne r0, psp                    \n\t"
   " mov r1, r0                       \n\t"
   " ldr r2, handler2_address_const   \n\t" /* Save stacked regs. */
   " ldr r3, [r1, 0]                  \n\t" /* r0  */
  " str r3, [r2, 0]                  \n\t"      
   " ldr r3, [r1, 4]                  \n\t" /* r1  */
  " str r3, [r2, 4]                  \n\t"  
   " ldr r3, [r1, 8]                  \n\t" /* r2  */
  " str r3, [r2, 8]                  \n\t"        
   " ldr r3, [r1, 12]                 \n\t" /* r3  */      
  " str r3, [r2, 12]                 \n\t"        
   " ldr r3, [r1, 16]                 \n\t" /* r12 */
  " str r3, [r2, 16]                 \n\t"
   " ldr r3, [r1, 20]                 \n\t" /* lr  */
  " str r3, [r2, 20]                 \n\t"
   " ldr r3, [r1, 24]                 \n\t" /* pc  */
  " str r3, [r2, 24]                 \n\t"
   " ldr r3, [r1, 28]                 \n\t" /* psr */
  " str r3, [r2, 28]                 \n\t"
   " str r4, [r2, 32]     \n\t" /* r4 */ /* Save remaining regs. */
   " str r5, [r2, 36]     \n\t" /* r5 */
   " str r6, [r2, 40]     \n\t" /* r6 */
   " str r7, [r2, 48]     \n\t" /* r7 */
   " str r8, [r2, 52]     \n\t" /* r8 */
   " str r9, [r2, 56]     \n\t" /* r9 */
   " str r10, [r2, 60]    \n\t" /* r10 */
   " str r11, [r2, 64]    \n\t" /* r11 */
   " ldr r2, handler5_address_const  \n\t" /* system registers dest */
   " ldr r3, handler6_address_const  \n\t" /* system registers source */
   " ldr r1, [r3, 0]        \n\t" /* 0xE000ED28 Status regs: two bytes:one half word  */
  " str r1, [r2, 0]        \n\t"      
   " ldr r1, [r3, 4]        \n\t" /* 0xE000ED2A HardFault Status (2 half word)  */
  " str r1, [r2, 4]        \n\t"      
   " ldr r1, [r3, 12]       \n\t" /* 0xE000ED34 MMAR: Mem Mgmnt Addr Reg  */
  " str r1, [r2, 12]       \n\t"      
   " ldr r1, [r3, 16]       \n\t" /* 0xE000ED38 BFAR: Bus Mgmnt Addr Reg    */
  " str r1, [r2, 16]       \n\t"      
   " movs r0, #111  \n\t"    /* Flash LEDs */
   " ldr r2, handler4_address_const   \n\t"
   " bx r2                            \n\t"
   " handler4_address_const: .word morse_trap \n\t"
   " handler2_address_const: .word reg_stack \n\t"
   " handler5_address_const: .word sys_regs \n\t"
   " handler6_address_const: .word 0xE000ED28 \n\t" /* Memory Mgmnt Fault Status Reg. */
 );

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    morse_trap(111);
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
morse_trap(333);
  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
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

/******************************************************************************/
/* STM32L4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32l4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles DMA1 channel1 global interrupt.
  */
void DMA1_Channel1_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel1_IRQn 0 */

  /* USER CODE END DMA1_Channel1_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_adc1);
  /* USER CODE BEGIN DMA1_Channel1_IRQn 1 */

  /* USER CODE END DMA1_Channel1_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel2 global interrupt.
  */
void DMA1_Channel2_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel2_IRQn 0 */
    // bmsspi.c
  bmsspi_spidmarx_IRQHandler(&hdma_spi1_rx);
  return;
  /* USER CODE END DMA1_Channel2_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_spi1_rx);
  /* USER CODE BEGIN DMA1_Channel2_IRQn 1 */

  /* USER CODE END DMA1_Channel2_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel3 global interrupt.
  */
void DMA1_Channel3_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel3_IRQn 0 */
  // bmsspi.c
  bmsspi_spidmatx_IRQHandler(&hdma_spi1_tx);
  return;
  /* USER CODE END DMA1_Channel3_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_spi1_tx);
  /* USER CODE BEGIN DMA1_Channel3_IRQn 1 */

  /* USER CODE END DMA1_Channel3_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel4 global interrupt.
  */
void DMA1_Channel4_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel4_IRQn 0 */

  /* USER CODE END DMA1_Channel4_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_i2c2_tx);
  /* USER CODE BEGIN DMA1_Channel4_IRQn 1 */

  /* USER CODE END DMA1_Channel4_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel5 global interrupt.
  */
void DMA1_Channel5_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Channel5_IRQn 0 */

  /* USER CODE END DMA1_Channel5_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_i2c2_rx);
  /* USER CODE BEGIN DMA1_Channel5_IRQn 1 */

  /* USER CODE END DMA1_Channel5_IRQn 1 */
}

/**
  * @brief This function handles TIM1 break interrupt and TIM15 global interrupt.
  */
void TIM1_BRK_TIM15_IRQHandler(void)
{
  /* USER CODE BEGIN TIM1_BRK_TIM15_IRQn 0 */
  //HAL_TIM_IRQHandler(&htim1);
  bmsspi_tim15_IRQHandler();
  return;
  /* USER CODE END TIM1_BRK_TIM15_IRQn 0 */
  HAL_TIM_IRQHandler(&htim1);
  HAL_TIM_IRQHandler(&htim15);
  /* USER CODE BEGIN TIM1_BRK_TIM15_IRQn 1 */

  /* USER CODE END TIM1_BRK_TIM15_IRQn 1 */
}

/**
  * @brief This function handles TIM1 update interrupt and TIM16 global interrupt.
  */
void TIM1_UP_TIM16_IRQHandler(void)
{
  /* USER CODE BEGIN TIM1_UP_TIM16_IRQn 0 */

  /* USER CODE END TIM1_UP_TIM16_IRQn 0 */
  HAL_TIM_IRQHandler(&htim1);
  HAL_TIM_IRQHandler(&htim16);
  /* USER CODE BEGIN TIM1_UP_TIM16_IRQn 1 */

  /* USER CODE END TIM1_UP_TIM16_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  /* USER CODE BEGIN USART1_IRQn 0 */

  /* USER CODE END USART1_IRQn 0 */
  HAL_UART_IRQHandler(&huart1);
  /* USER CODE BEGIN USART1_IRQn 1 */

  /* USER CODE END USART1_IRQn 1 */
}

/**
  * @brief This function handles DMA2 channel6 global interrupt.
  */
void DMA2_Channel6_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Channel6_IRQn 0 */

  /* USER CODE END DMA2_Channel6_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart1_tx);
  /* USER CODE BEGIN DMA2_Channel6_IRQn 1 */

  /* USER CODE END DMA2_Channel6_IRQn 1 */
}

/**
  * @brief This function handles DMA2 channel7 global interrupt.
  */
void DMA2_Channel7_IRQHandler(void)
{
  /* USER CODE BEGIN DMA2_Channel7_IRQn 0 */

  /* USER CODE END DMA2_Channel7_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_usart1_rx);
  /* USER CODE BEGIN DMA2_Channel7_IRQn 1 */

  /* USER CODE END DMA2_Channel7_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
