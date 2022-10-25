/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/*Command line to load program over CAN
:~~/GliderWinchCommons/embed/svn_discoveryf4/PC/sensor/CANldr/trunk$ ./CANldr 127.0.0.1 32123 B0A00000 ~/GliderWinchItems/BMS/bmsadbms1818/build/bms1818.xbin; echo $?
*/
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include "morse.h"
#include "DTW_counter.h"
#include "fetonoff.h"

#include "adcparams.h"
#include "SerialTaskSend.h"
#include "SerialTaskReceive.h"
#include "CanTask.h"
#include "can_iface.h"
#include "canfilter_setup.h"
#include "getserialbuf.h"
#include "yprintf.h"
#include "DTW_counter.h"
#include "BQTask.h"
#include "bqview.h"
#include "bqcellbal.h"
#include "bq_func_init.h"
#include "bq_items.h"
#include "ADCTask.h"
#include "MailboxTask.h"
#include "CanCommTask.h"
#include "BMSTask.h"
#include "fanop.h"
#include "chgr_items.h"
#include "bmsspi.h"
#include "bms_items.h"
#include "rtcregs.h"

#include "FreeRTOS.h"
#include "semphr.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
uint32_t timectr = 0;
struct CAN_CTLBLOCK* pctl0; // Pointer to CAN1 control block
//struct CAN_CTLBLOCK* pctl1; // Pointer to CAN2 control block

uint32_t debugTX1b;
uint32_t debugTX1b_prev;

uint32_t debugTX1c;
uint32_t debugTX1c_prev;

uint32_t debug03;
uint32_t debug03_prev;

//extern osThreadId SerialTaskHandle;
//extern osThreadId CanTxTaskHandle;
//extern osThreadId CanRxTaskHandle;
//extern osThreadId SerialTaskReceiveHandle;

uint16_t m_trap = 450; // Trap codes for MX Init() and Error Handler

uint8_t canflag;
uint8_t canflag1;
//uint8_t canflag2;

int8_t rtcregs_ret; // RTC register init return

//const uint32_t i_am_canid = I_AM_CANID;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
  /* 
  Timer usage
                         1    2   3   4   auto  auto
  TIM1 charger control  PWM   x   x   x  comp1 comp2
                        PA8               PA6   PA11 
  TIM2 FAN PWM TACH     PWM   x  IC   x    
                        PA5     PB10
  TIM7  System          x     

  TIM15 SPI/ADC/BMS     OC    x
  
  */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

CAN_HandleTypeDef hcan1;

COMP_HandleTypeDef hcomp1;
COMP_HandleTypeDef hcomp2;

DAC_HandleTypeDef hdac1;

I2C_HandleTypeDef hi2c2;
DMA_HandleTypeDef hdma_i2c2_rx;
DMA_HandleTypeDef hdma_i2c2_tx;

OPAMP_HandleTypeDef hopamp1;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_rx;
DMA_HandleTypeDef hdma_spi1_tx;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim15;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 384 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE BEGIN PV */
uint16_t errorcode;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_CAN1_Init(void);
static void MX_RTC_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_COMP1_Init(void);
static void MX_DAC1_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM15_Init(void);
static void MX_OPAMP1_Init(void);
static void MX_SPI1_Init(void);
static void MX_I2C2_Init(void);
static void MX_COMP2_Init(void);
void StartDefaultTask(void *argument);

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
  BaseType_t ret;    // Used for returns from function calls
  osThreadId Thrdret;  // Return from thread create
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  DTW_counter_init();
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
#define NUMCIRBCB1  16 // Size of circular buffer of BCB for usart3
  ret = xSerialTaskSendAdd(&HUARTMON, NUMCIRBCB1, 1); // dma
  if (ret < 0) morse_trap(111); // Panic LED flashing
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_CAN1_Init();
  MX_RTC_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_COMP1_Init();
  MX_DAC1_Init();
  MX_TIM1_Init();
  MX_TIM15_Init();
  MX_OPAMP1_Init();
  MX_SPI1_Init();
  MX_I2C2_Init();
  MX_COMP2_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* Setup semaphore for yprint and sprintf et al. */
  yprintf_init();
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */

  Thrdret = xADCTaskCreate(osPriorityNormal+2); // (arg) = priority
  if (Thrdret == NULL) morse_trap(117);

  /* Create serial task (priority) */
  // Task handle "osThreadId SerialTaskHandle" is global
  Thrdret = xSerialTaskSendCreate(osPriorityNormal); // Create task and set Task priority
  if (Thrdret == NULL) morse_trap(112);

    /* Add bcb circular buffer to SerialTaskSend for usart3 -- PC monitor */
  #define NUMCIRBCB3  16 // Size of circular buffer of BCB for usart3
  ret = xSerialTaskSendAdd(&HUARTMON, NUMCIRBCB3, 1); // dma
  if (ret < 0) morse_trap(14); // Panic LED flashing

  /* Setup TX linked list for CAN  */
   // CAN1 (CAN_HandleTypeDef *phcan, uint8_t canidx, uint16_t numtx, uint16_t numrx);
  pctl0 = can_iface_init(&hcan1, 0, 64, 32);       
  if (pctl0 == NULL) morse_trap(118); // Panic LED flashing
  if (pctl0->ret < 0) morse_trap(119);

  /* Create serial receiving task. */
  //ret = xSerialTaskReceiveCreate(osPriorityNormal);
  //if (ret != pdPASS) morse_trap(113);

  /* BQTask. */
//  TaskHandle_t retT = xBQTaskCreate(osPriorityNormal);
//  if (retT == NULL) morse_trap(114);

  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  // Initialize struct for BQ. */
  bq_func_init();

  /* definition and creation of CanTxTask - CAN driver TX interface. */
  QueueHandle_t QHret = xCanTxTaskCreate(osPriorityNormal, 48); // CanTask priority, Number of msgs in queue
  if (QHret == NULL) morse_trap(120); // Panic LED flashing

  /* definition and creation of CanRxTask - CAN driver RX interface. */
  /* The MailboxTask takes care of the CANRx                         */
//  Qidret = xCanRxTaskCreate(1, 32); // CanTask priority, Number of msgs in queue
//  if (Qidret < 0) morse_trap(6); // Panic LED flashing

  /* Create MailboxTask */
  xMailboxTaskCreate(osPriorityNormal); // (arg) = priority

  /* Create Mailbox control block w 'take' pointer for each CAN module. */
  struct MAILBOXCANNUM* pmbxret;
  // (CAN1 control block pointer, size of circular buffer)
  pmbxret = MailboxTask_add_CANlist(pctl0, 32);
  if (pmbxret == NULL) morse_trap(215);

  /* Setup CAN hardware filters to default to accept all ids. */
  HAL_StatusTypeDef Cret;
  Cret = canfilter_setup_first(0, &hcan1, 15); // CAN1
  if (Cret == HAL_ERROR) morse_trap(122);

  TaskHandle_t retT = xBMSTaskCreate(osPriorityNormal+1);  
  if (retT == NULL) morse_trap(123);

  /* CAN communication */
  retT = xCanCommCreate(osPriorityNormal+1);
  if (retT == NULL) morse_trap(121);

    /* Select interrupts for CAN1 */
  HAL_CAN_ActivateNotification(&hcan1, \
    CAN_IT_TX_MAILBOX_EMPTY     |  \
    CAN_IT_RX_FIFO0_MSG_PENDING |  \
    CAN_IT_RX_FIFO1_MSG_PENDING    );

  /* Init some things used by tasks. */
  bq_func_init();
  chgr_items_init();
  fanop_init();
  bmsspi_preinit();
  rtcregs_ret = rtcregs_init();

  /* This will run the 5 and 3.3v regulators from the ribbon
     Cell #3 when the CAN power has been removed. */
  HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_SET); // ON
//HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13,GPIO_PIN_RESET); // OFF

  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV8;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

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
  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.NbrOfConversion = 9;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = ENABLE;
  hadc1.Init.Oversampling.Ratio = ADC_OVERSAMPLING_RATIO_16;
  hadc1.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_NONE;
  hadc1.Init.Oversampling.TriggeredMode = ADC_TRIGGEREDMODE_SINGLE_TRIGGER;
  hadc1.Init.Oversampling.OversamplingStopReset = ADC_REGOVERSAMPLING_CONTINUED_MODE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  sConfig.SamplingTime = ADC_SAMPLETIME_24CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_12;
  sConfig.Rank = ADC_REGULAR_RANK_4;
  sConfig.SamplingTime = ADC_SAMPLETIME_640CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_14;
  sConfig.Rank = ADC_REGULAR_RANK_5;
  sConfig.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Rank = ADC_REGULAR_RANK_6;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_VREFINT;
  sConfig.Rank = ADC_REGULAR_RANK_7;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = ADC_REGULAR_RANK_8;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = ADC_REGULAR_RANK_9;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 4;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_5TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = ENABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

}

/**
  * @brief COMP1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_COMP1_Init(void)
{

  /* USER CODE BEGIN COMP1_Init 0 */
  errorcode = 998; // Error_Handler morse_trap code

  /* USER CODE END COMP1_Init 0 */

  /* USER CODE BEGIN COMP1_Init 1 */

  /* USER CODE END COMP1_Init 1 */
  hcomp1.Instance = COMP1;
  hcomp1.Init.InvertingInput = COMP_INPUT_MINUS_DAC1_CH1;
  hcomp1.Init.NonInvertingInput = COMP_INPUT_PLUS_IO2;
  hcomp1.Init.OutputPol = COMP_OUTPUTPOL_NONINVERTED;
  hcomp1.Init.Hysteresis = COMP_HYSTERESIS_NONE;
  hcomp1.Init.BlankingSrce = COMP_BLANKINGSRC_NONE;
  hcomp1.Init.Mode = COMP_POWERMODE_HIGHSPEED;
  hcomp1.Init.WindowMode = COMP_WINDOWMODE_DISABLE;
  hcomp1.Init.TriggerMode = COMP_TRIGGERMODE_NONE;
  if (HAL_COMP_Init(&hcomp1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN COMP1_Init 2 */

  /* USER CODE END COMP1_Init 2 */

}

/**
  * @brief COMP2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_COMP2_Init(void)
{

  /* USER CODE BEGIN COMP2_Init 0 */
  errorcode = 997; // Error_Handler morse_trap code

  /* USER CODE END COMP2_Init 0 */

  /* USER CODE BEGIN COMP2_Init 1 */

  /* USER CODE END COMP2_Init 1 */
  hcomp2.Instance = COMP2;
  hcomp2.Init.InvertingInput = COMP_INPUT_MINUS_DAC1_CH2;
  hcomp2.Init.NonInvertingInput = COMP_INPUT_PLUS_IO2;
  hcomp2.Init.OutputPol = COMP_OUTPUTPOL_NONINVERTED;
  hcomp2.Init.Hysteresis = COMP_HYSTERESIS_NONE;
  hcomp2.Init.BlankingSrce = COMP_BLANKINGSRC_NONE;
  hcomp2.Init.Mode = COMP_POWERMODE_HIGHSPEED;
  hcomp2.Init.WindowMode = COMP_WINDOWMODE_DISABLE;
  hcomp2.Init.TriggerMode = COMP_TRIGGERMODE_NONE;
  if (HAL_COMP_Init(&hcomp2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN COMP2_Init 2 */

  /* USER CODE END COMP2_Init 2 */

}

/**
  * @brief DAC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC1_Init(void)
{

  /* USER CODE BEGIN DAC1_Init 0 */

  /* USER CODE END DAC1_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC1_Init 1 */

  /* USER CODE END DAC1_Init 1 */
  /** DAC Initialization
  */
  hdac1.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1) != HAL_OK)
  {
    Error_Handler();
  }
  /** DAC channel OUT1 config
  */
  sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_DISABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_ENABLE;
  sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /** DAC channel OUT2 config
  */
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC1_Init 2 */

  /* USER CODE END DAC1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = 0x00303D5B;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief OPAMP1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_OPAMP1_Init(void)
{

  /* USER CODE BEGIN OPAMP1_Init 0 */

  /* USER CODE END OPAMP1_Init 0 */

  /* USER CODE BEGIN OPAMP1_Init 1 */

  /* USER CODE END OPAMP1_Init 1 */
  hopamp1.Instance = OPAMP1;
  hopamp1.Init.PowerSupplyRange = OPAMP_POWERSUPPLY_HIGH;
  hopamp1.Init.Mode = OPAMP_PGA_MODE;
  hopamp1.Init.NonInvertingInput = OPAMP_NONINVERTINGINPUT_IO0;
  hopamp1.Init.InvertingInput = OPAMP_INVERTINGINPUT_IO0;
  hopamp1.Init.PgaGain = OPAMP_PGA_GAIN_8;
  hopamp1.Init.PowerMode = OPAMP_POWERMODE_NORMALPOWER;
  hopamp1.Init.UserTrimming = OPAMP_TRIMMING_FACTORY;
  if (HAL_OPAMP_Init(&hopamp1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN OPAMP1_Init 2 */

  /* USER CODE END OPAMP1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */
  errorcode = 991; // Error_Handler morse_trap code
  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIMEx_BreakInputConfigTypeDef sBreakInputConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */
  errorcode = 992; // Error_Handler morse_trap code
  
  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 79;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakInputConfig.Source = TIM_BREAKINPUTSOURCE_BKIN;
  sBreakInputConfig.Enable = TIM_BREAKINPUTSOURCE_ENABLE;
  sBreakInputConfig.Polarity = TIM_BREAKINPUTSOURCE_POLARITY_HIGH;
  if (HAL_TIMEx_ConfigBreakInput(&htim1, TIM_BREAKINPUT_BRK, &sBreakInputConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakInputConfig.Source = TIM_BREAKINPUTSOURCE_COMP2;
  if (HAL_TIMEx_ConfigBreakInput(&htim1, TIM_BREAKINPUT_BRK, &sBreakInputConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakInputConfig.Source = TIM_BREAKINPUTSOURCE_BKIN;
  if (HAL_TIMEx_ConfigBreakInput(&htim1, TIM_BREAKINPUT_BRK2, &sBreakInputConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakInputConfig.Source = TIM_BREAKINPUTSOURCE_COMP1;
  if (HAL_TIMEx_ConfigBreakInput(&htim1, TIM_BREAKINPUT_BRK2, &sBreakInputConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_ENABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_ENABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_ENABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_ENABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_ENABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */
  
  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */
  errorcode = 993; // Error_Handler morse_trap code
  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */
  errorcode = 994; // Error_Handler morse_trap code

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);

}

/**
  * @brief TIM15 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM15_Init(void)
{

  /* USER CODE BEGIN TIM15_Init 0 */
  errorcode = 995; // Error_Handler morse_trap code

  /* USER CODE END TIM15_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM15_Init 1 */

  /* USER CODE END TIM15_Init 1 */
  htim15.Instance = TIM15;
  htim15.Init.Prescaler = 16;
  htim15.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim15.Init.Period = 65535;
  htim15.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim15.Init.RepetitionCounter = 0;
  htim15.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim15) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim15, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OC_Init(&htim15) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim15, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_OC_ConfigChannel(&htim15, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim15, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM15_Init 2 */

  /* USER CODE END TIM15_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */
  errorcode = 996; // Error_Handler morse_trap code

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_8;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  /* DMA1_Channel2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);
  /* DMA2_Channel6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Channel6_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(DMA2_Channel6_IRQn);
  /* DMA2_Channel7_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Channel7_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(DMA2_Channel7_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, RTC_BAT_PWR_Pin|GPIO_PIN_4|DUMP2_Pin|DUMP_NOT_Pin
                          |DUMP_Pin|HEATER_NOT_Pin|HEATER_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED_GRN_Pin|LED_RED_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, PADxx_Pin|PAD7_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CSB_GPIO_Port, CSB_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BQ_LD_GPIO_Port, BQ_LD_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : RTC_BAT_PWR_Pin PC4 DUMP_NOT_Pin DUMP_Pin
                           HEATER_NOT_Pin HEATER_Pin */
  GPIO_InitStruct.Pin = RTC_BAT_PWR_Pin|GPIO_PIN_4|DUMP_NOT_Pin|DUMP_Pin
                          |HEATER_NOT_Pin|HEATER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LED_GRN_Pin LED_RED_Pin PADxx_Pin PAD7_Pin */
  GPIO_InitStruct.Pin = LED_GRN_Pin|LED_RED_Pin|PADxx_Pin|PAD7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : DUMP2_Pin */
  GPIO_InitStruct.Pin = DUMP2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DUMP2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CSB_Pin */
  GPIO_InitStruct.Pin = CSB_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(CSB_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BQ_LD_Pin */
  GPIO_InitStruct.Pin = BQ_LD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BQ_LD_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : WDT_Pin */
  GPIO_InitStruct.Pin = WDT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(WDT_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
uint32_t adc1_ctr_prev;
uint32_t fctr = 0;
uint8_t adcctr = 0;
uint8_t wakectr;
uint8_t state_defaultTask = 0;
static struct BMSREQ_Q bmsreq_1;
struct BMSREQ_Q* pbmsreq_1 = &bmsreq_1;

char* pcheader =
"\n\r                 1      2      3      4      5      6      7      8      9     10     11     12     13     14     15     16     17     18";

/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  int32_t i;
  uint32_t mctr = 0;

// Poll 'done' so task handle not needed
  //bmsreq_read.bmsTaskHandle   = xTaskGetCurrentTaskHandle();
  //bmsreq_update.bmsTaskHandle = bmsreq_read.msTaskHandle;

  struct SERIALSENDTASKBCB* pbuf1 = getserialbuf(&HUARTMON,144);
  if (pbuf1 == NULL) morse_trap(115);
  struct SERIALSENDTASKBCB* pbuf2 = getserialbuf(&HUARTMON,128);
  if (pbuf2 == NULL) morse_trap(125);
//struct SERIALSENDTASKBCB* pbuf3 = getserialbuf(&HUARTMON,128);
//if (pbuf3 == NULL) morse_trap(125);
//struct SERIALSENDTASKBCB* pbuf4 = getserialbuf(&HUARTMON,128);
//if (pbuf4 == NULL) morse_trap(125);

  yprintf(&pbuf1,"\n\n\rPROGRAM STARTS: rtcregs_ret: %d",rtcregs_ret);

  /* The 'for' loop polls. The following times the loop. */
  #define MAINFORLOOPDELAY 50 // Delay of 'for' loop in ms
  const TickType_t xPeriod = pdMS_TO_TICKS(MAINFORLOOPDELAY);  
  TickType_t tickcnt = xTaskGetTickCount();
  
  bq_items_init(); // Updates tickcnt

//extern uint32_t bmsdbctr;
//uint32_t bmsdbctr_prev = bmsdbctr;
extern uint8_t dbgka;
uint8_t dbgka_prev = dbgka;
extern uint32_t bshift;  
extern uint32_t dbstat2;

/* Testint discharge FET bits. */
#ifdef TEST_WALK_DISCHARGE_FET_BITS // See main.h
extern uint8_t dischgfet; // Test fet bit (0-17)
  uint8_t  dischgfet_ctr = 20; 
extern  dbgcancommloop;
uint32_t dbgcancommloop_prev;
#endif


  for(;;) /* Loop polls various operations. */
  {  
    vTaskDelayUntil( &tickcnt, xPeriod );

#if 1 // Processor ADC display
    adcctr += 1;
    if (adcctr > 20)
    {
      adcctr = 0;
extern uint32_t dwtdiff;      
  #if 1
      yprintf(&pbuf1,"\n\rADCf: %4d %6d:",(adc1.ctr-adc1_ctr_prev),dwtdiff);
      adc1_ctr_prev = adc1.ctr;
      for (i = 0; i < 9; i++)
      {
        yprintf(&pbuf2," %6.4f",adc1.abs[i].filt);
      }
      float tcf = adcparams_caltemp();
      yprintf(&pbuf1," temp: %4.1f",tcf);
  #endif 

  #if 0     
      float tcf1 = adcparams_caltemp();
      yprintf(&pbuf1,"\n\r temp: %6.4f vref %0.5f cal1 %0.1f cal2 %0.1f caldiff %0.4f calrate %0.5f",tcf1,
adc1.common.ts_vref,
adc1.common.ts_cal1,      /* (float)(*PTS_CAL1); // Factory calibration */
adc1.common.ts_cal2,      /* (float)(*PTS_CAL2); // Factory calibration */
adc1.common.ts_caldiff,   /* (padccommon->ts_cal2 - padccommon->ts_cal1); */
adc1.common.ts_calrate );

  #endif      

  #if 1
      yprintf(&pbuf1,"\n\rADCi: %4d %6d",(adc1.ctr-adc1_ctr_prev),dwtdiff);     
    extern uint32_t dbg_adcsum[ADCDIRECTMAX];        
      yprintf(&pbuf1," %6d %6d %6d %6d %6d %6d %6d %6d %6d",
          dbg_adcsum[0],dbg_adcsum[1],dbg_adcsum[2],dbg_adcsum[3],dbg_adcsum[4],
          dbg_adcsum[5],dbg_adcsum[6],dbg_adcsum[7],dbg_adcsum[8]);
  #endif 
      adc1_ctr_prev = adc1.ctr;
    }
#endif    

#if 1
//    bq_items();
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET); // GRN LED
    if (dbgka_prev != dbgka)
    {
      dbgka_prev = dbgka;
      HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_RESET); // GRN LED

      switch (dbgka)
      {
      case 0: // config regs
       yprintf(&pbuf1,"\n\n\r\t\t%5d",mctr++);

       yprintf(&pbuf2,"\n\r%5d CFGR   %04X %04X %04X : %04X %04X %04X %d",dbgka,
          bmsspiall.configreg[0],bmsspiall.configreg[1],bmsspiall.configreg[2],
          bmsspiall.configreg[3],bmsspiall.configreg[4],bmsspiall.configreg[5],bshift+1);
        break;

      case 1: // Stat regs
       yprintf(&pbuf2,"\n\r%5d STAT   %04X %04X %04X : %04X %04X %04X %d ",dbgka,
          bmsspiall.statreg[0],bmsspiall.statreg[1],bmsspiall.statreg[2],
          bmsspiall.statreg[3],bmsspiall.statreg[4],bmsspiall.statreg[5],dbstat2/16);      
        break;

      case 2:
       yprintf(&pbuf2,"\n\r%5d SREG   %04X %04X %04X",dbgka,
          bmsspiall.sreg[0],bmsspiall.sreg[1],bmsspiall.sreg[2]);
        break;

      case 3:
       yprintf(&pbuf2,"\n\r%5d WFGR   %04X %04X %04X : %04X %04X %04X",dbgka,
          bmsspiall.configreg[0],bmsspiall.configreg[1],bmsspiall.configreg[2],
          bmsspiall.configreg[3],bmsspiall.configreg[4],bmsspiall.configreg[5]);
        break;

      case 4:
       yprintf(&pbuf1,"%s",pcheader);
       yprintf(&pbuf2,"\n\r%5d ADCVAX",dbgka);
       for (i = 0; i < 18; i++) yprintf(&pbuf1," %6d",bmsspiall.cellreg[i]);
       yprintf(&pbuf1," %d",dbstat2/16);
        break;

      case 5: // Temperature (AUX reg)
       yprintf(&pbuf2,"\n\r%5d AUX    %6d %6d %6d %6d",dbgka,
        bmsspiall.auxreg[0],bmsspiall.auxreg[1],bmsspiall.auxreg[2],bmsspiall.auxreg[3]);
       yprintf(&pbuf1," %6d %6d %6d %6d %6d %d",
        bmsspiall.auxreg[4],bmsspiall.auxreg[5],bmsspiall.auxreg[6],bmsspiall.auxreg[7],bmsspiall.auxreg[8],
        dbstat2/16);

      bms_items_extract_statreg();
      yprintf(&pbuf2,"\n\r%5d        TS:%6.2f TMP:%5.1f VA:%6.3f VG:%6.3f",dbgka,
          extractstatreg.sc, extractstatreg.itmp,
          extractstatreg.va, extractstatreg.vd  );

      bms_items_therm_temps(); // Convert thermistor readings to tempature (deg C)
      extern float fanrpm;
        yprintf(&pbuf1,"\n\rTEMP:%5.1f %5.1f %5.1f Fanpwm: %d Fanrpm: %0.3f",
          bqfunction.lc.thermcal[0].temp,
          bqfunction.lc.thermcal[1].temp,
          bqfunction.lc.thermcal[2].temp, 
          bqfunction.fanspeed, fanrpm);
        break;

      default:
        yprintf(&pbuf2,"\n\r%d %5d BOGUS",dbgka, mctr);
        break;
      }
    }
#endif    

#if 1
    /* Update FAN control. (4/sec) */
    bms_items_therm_temps(); // Convert thermistor readings to tempature (deg C)
    fanop();
#endif

#if 0
/* Check WAKE */


#endif

/* Testint discharge FET bits. */
#ifdef TEST_WALK_DISCHARGE_FET_BITS // See main.h
  if (dischgfet_ctr++ > 3)
  {
    yprintf(&pbuf1,"\n\r\t\t\t\t\t\t### TEST: WALK DISCHARGE FET #%02u %u\n\r",(dischgfet+1),dbgcancommloop-dbgcancommloop_prev);
    dischgfet_ctr = 0;
    dbgcancommloop_prev = dbgcancommloop;
  }
#endif

#ifndef TEST_WALK_DISCHARGE_FET_BITS 
//#if 1
    /* Cell balance & control. */
    uint32_t dcc = extractconfigreg.dcc;
    #define LSPC 7 // column spacing
    char cline[LSPC*18+2];

    /* Check cell balance. */
    uint8_t ret8 = bq_items(); 
    if (ret8 == 2)
    { // Here a cell balance update was completed
      bms_items_extract_configreg();
extern uint8_t dbgf;
extern struct BMSREQ_Q  bmstask_q_readbms;
//      yprintf(&pbuf1,"\n\r    %05X   ",bmstask_q_readbms.setfets);
//      for (i=0; i < 18; i++) yprintf(&pbuf2,"%4d",(1+i));
      yprintf(&pbuf1,"%s",pcheader);

      yprintf(&pbuf2,"\n\r%5d %02d FETS    ",fctr++,dbgf+1);
      memset(cline,' ',(LSPC*18));
      // Build a nice ASCII line whilst previous line prints
      for (i=0; i < 18; i++)
      {
        if ((dcc & (1<<i)) != 0)
          cline[i*LSPC] = '#';
        else
          cline[i*LSPC] = '.';
      }
      cline[18*LSPC] = 0;
      yprintf(&pbuf1,"%s",cline);

int32_t csum = 0;
      yprintf(&pbuf2,"\n\rcellv[i] : ");
      for (i = 0; i < 18; ++i)
      {
        csum += bqfunction.cellv[i];
        yprintf(&pbuf2," %6.0f",bqfunction.cellv[i]);
      }
      yprintf(&pbuf1," %d",csum);

      extern uint32_t dbgcell[18];
      yprintf(&pbuf2,"\n\rdbgcellv : ");
      for (i = 0; i < 18; ++i) yprintf(&pbuf2," %6d",dbgcell[i]);

      yprintf(&pbuf2,"\n\r%5d %02d DCHG    ",fctr++,dbgf+1);
      memset(cline,' ',(LSPC*18));
      // Build a nice ASCII line whilst previous line prints
      for (i=0; i < 18; i++)
      {
        if ((dcc & (1<<i)) != 0)
          cline[i*LSPC] = '@';
        else
          cline[i*LSPC] = '.';
      }
      cline[18*LSPC] = 0;
      yprintf(&pbuf1,"%s",cline);        

      yprintf(&pbuf1,"\n\r%5d %02d MAX|MIN ",fctr++,dbgf+1);
      memset(cline,' ',(LSPC*18));
      // Build a nice ASCII line whilst previous line prints
      for (i=0; i < 18; i++)
      {
        cline[i*LSPC] = '.';
        if ((bqfunction.cellv_max_bits & (1<<i)) != 0)
          cline[i*LSPC] = '+';
        if ((bqfunction.cellv_min_bits & (1<<i)) != 0)
          cline[i*LSPC] = '-';
      }
      cline[18*LSPC] = 0;
      yprintf(&pbuf2,"%s",cline);  
extern uint32_t dbgtrc;
      yprintf(&pbuf1,"\n\rFETSTAT: 0x%02X  battery_status: 0x%02X dbgtrc: 0x%04x ",bqfunction.fet_status,bqfunction.battery_status,dbgtrc);     

#if 0  
      yprintf(&pbuf1,"\n\rcellv_latest[i]: ");
      for (i = 0; i < 18; ++i) yprintf(&pbuf2," %5d",bqfunction.cellv_latest[i]);
#endif
      yprintf(&pbuf2,"\n\rcellspresent: %05X",bqfunction.cellspresent); 

      yprintf(&pbuf1,"\n\rcellv_hi: x %2d v %5d",bqfunction.cellx_high,bqfunction.cellv_high);
      yprintf(&pbuf2,"\n\rcellv_lo: x %2d v %5d",bqfunction.cellx_low, bqfunction.cellv_low);
      yprintf(&pbuf1,"\n\rhyster_sw: %d hysterbits lo %05X",bqfunction.hyster_sw,bqfunction.hysterbits_lo);
      yprintf(&pbuf1,"\n\rhysterv_lo:%5d",bqfunction.hysterv_lo);
      yprintf(&pbuf2,"\n\rcellv_max: %5d cellv_max: %05X",bqfunction.lc.cellv_max,bqfunction.cellv_max_bits);
      yprintf(&pbuf1,"\n\rcellv_min: %5d cellv_min: %05X",bqfunction.lc.cellv_min,bqfunction.cellv_min_bits);

      yprintf(&pbuf2,"\n\rcellv_vls: %5d cellv_vlc_bits: %05X",bqfunction.lc.cellv_vlc, bqfunction.cellv_vlc_bits);

      yprintf(&pbuf1,"\n\rcallbal:   %05X  fet_status: %04X FET_CHRG: %01X",bqfunction.cellbal,
          bqfunction.fet_status,(bqfunction.fet_status & FET_CHGR));

      extern uint32_t dbgcellbal;
      yprintf(&pbuf1,"\n\rdbgcellbal:%05X",dbgcellbal);

      yprintf(&pbuf1,"\n\r config:   %04X %04X %04X %04X %04X %04X",
        bmsspiall.configreg[0],bmsspiall.configreg[1],bmsspiall.configreg[2],
        bmsspiall.configreg[3],bmsspiall.configreg[4],bmsspiall.configreg[5]);

    if (bmsspiall.err1ct != 0)
    {
      yprintf(&pbuf1,"\n\n\r### LOOPCTR ERR: %d\n\r",bmsspiall.err1ct);
      bmsspiall.err1ct = 0;
    }      
extern uint32_t dbgCanTask1;
yprintf(&pbuf2,"\n\rdbCanTask1: %d", dbgCanTask1);

  #if 0 /* RTC register check. */
      switch (state_defaultTask)
      {
        case 0:
          bmsreq_1.reqcode = REQ_RTCREAD;
          bmsreq_1.done = 1; // Show req is queued
          int ret = xQueueSendToBack(BMSTaskReadReqQHandle, &pbmsreq_1, 0);
          if (ret != pdPASS) morse_trap(109);      
          state_defaultTask = 1;
        case 1:          
          if (bmsreq_1.done != 0) 
            break;
          uint32_t* prtc32 = (uint32_t*)&RTC->BKP0R;
          yprintf(&pbuf1,"\n\rRTC: F:%05X H:%05X L:%05X",*(prtc32+0),*(prtc32+1),*(prtc32+2));
          
          uint16_t* prtc16 = (uint16_t*)&RTC->BKP0R + 6;
          for (i = 0; i < 18; i++)
          {
            yprintf(&pbuf2," %5d",*prtc16);
            prtc16 += 1;
          }

          uint8_t* prtc8 = (uint8_t*)prtc16;
          yprintf(&pbuf1," H:%02X B:%02X F:%02X E:%02X ",*(prtc8+0),*(prtc8+1),*(prtc8+2),*(prtc8+3));

          if (bmsreq_1.other == 0)
          {
            yprintf(&pbuf2," OK");
          }
          else
          {
            yprintf(&pbuf2," NOT OK");
            /* if NOT OK do an update. */
            bmsreq_1.reqcode = REQ_RTCUPDATE;
            bmsreq_1.done = 1; // Show req is queued
            int ret = xQueueSendToBack(BMSTaskReadReqQHandle, &pbmsreq_1, 0);
            if (ret != pdPASS) morse_trap(108);      
            state_defaultTask = 2;
          }
        case 2:
          if (bmsreq_1.done != 0) 
            break;
          state_defaultTask = 0;
          break;
      }         
  #endif

    }
#endif    
  } 



  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM16 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM16) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  morse_trap(2222);
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
  morse_trap(3333);
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
