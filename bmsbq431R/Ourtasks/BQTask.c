/******************************************************************************
* File Name          : BQTask.c
* Date First Issued  : 09/08/2021
* Description        : BQ76952 BMS w STM32CubeMX w FreeRTOS
*******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "malloc.h"

#include "main.h"
#include "morse.h"

#include "BQTask.h"
#include "BQ769x2Header.h"

extern I2C_HandleTypeDef hi2c1;

static uint8_t bqconfig(void);
static uint8_t subcmdcom(uint8_t* pr, uint8_t nr, uint16_t cmd, uint16_t data, uint8_t nd);
static uint8_t cmdcom(uint8_t* pr, uint8_t nr, uint8_t cmd);
static void morse_sos(void);
static uint8_t readblock_cmd(uint8_t* pr, uint8_t nr, uint8_t cmd);
static uint8_t getcellv(void);

uint8_t bqchksum(uint8_t* ptr, uint8_t len);

struct BQFUNCTION bqfunction;

TaskHandle_t BQTaskHandle = NULL;

 uint8_t bufcmd[4];
 uint8_t bufrcv[4];
 uint8_t bqflag = 0;
 //uint8_t bqbuf[34];

union BQFIELD2
{
	uint8_t   u8[2];
	uint16_t u16;
	int8_t    s8[2];
	int16_t  s16;
};


union BQFIELD4
{
	uint8_t   u8[4];
	uint16_t u16[2];
	uint32_t u32;
	int8_t    s8[4];
	int16_t  s16[2];
	int32_t  s32;
};

 struct SUBCOMMAND
 {
 	union BQFIELD4 cmd4; // Command to BQ
 	union BQFIELD4 rcv4; // BQ response
 };

 int16_t cellv[2][BQVSIZE];
 uint8_t cvidx = 0;
 uint32_t cvflag = 0;

uint16_t device_number_u16;
uint16_t battery_status;
uint16_t reg0_config_u16;
uint16_t ddsgp_config_u16;

I2C_HandleTypeDef hi2c1;

/* *************************************************************************
 * static void brake_init(struct BQFUNCTION* p);
 *	@brief	: Some initialization before endless loop starts
 * *************************************************************************/
static void bq_init(void)
{
	/* Checks if target device is ready for communication. */
	/* 3 is number of trials, 1000ms is timeout */
//	if (HAL_I2C_IsDeviceReady(&hi2c1, 0x11, 3, 1000) != HAL_OK) 
//		morse_trap(331);
	
	return;
}
/* *************************************************************************
 * void StartBQTask(void const * argument);
 *	@brief	: Task startup
 * *************************************************************************/
void StartBQTask(void* argument)
{
	osDelay(250); // Wait for BQ to initialize/reset

//	bq_init();

//	while (bqconfig() != 0) 
//	bqconfig();
//	{
//		morse_sos();
//	}

	for (;;)
	{
	//	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_RESET); // RED LED ON
	//	osDelay(50);
	//	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET); // RED LED OFF
		osDelay(1000);

HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5, GPIO_PIN_SET); // RST
HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET); // PADxx
HAL_GPIO_WritePin(GPIOB,GPIO_PIN_15,GPIO_PIN_SET); // PAD7

HAL_GPIO_WritePin(GPIOC,GPIO_PIN_6,GPIO_PIN_SET);   // Dump2

HAL_GPIO_WritePin(GPIOC,GPIO_PIN_8,GPIO_PIN_RESET); // Not Dump
HAL_GPIO_WritePin(GPIOC,GPIO_PIN_10,GPIO_PIN_SET);  // Dump

HAL_GPIO_WritePin(GPIOC,GPIO_PIN_11,GPIO_PIN_SET);   // Heater
HAL_GPIO_WritePin(GPIOC,GPIO_PIN_12,GPIO_PIN_RESET); // Not heater

HAL_GPIO_WritePin(GPIOD,GPIO_PIN_0, GPIO_PIN_SET);

HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,GPIO_PIN_SET); // PAD7
HAL_GPIO_WritePin(GPIOB,GPIO_PIN_7,GPIO_PIN_SET); // PAD7



	osDelay(1000);

HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_5); // RST
HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_12); // PADxx
HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_15); // PAD7

HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_6);   // Dump2

HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_8); // Not Dump
HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_10);  // Dump

HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_11);   // Heater
HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_12); // Not heater

HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_0);

HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_6); // RST
HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_7); // RST


//		while(cmdcom((uint8_t *)&battery_status, 2, BatteryStatus) != 0)
//		{
			morse_sos();
//			osDelay(10);
//		}

//		getcellv();
//		if (bqflag != 0)
//		{ /* Reconfigure if communication failed. */
//			while (bqconfig() != 0) 
//			{
//				morse_sos();
//				osDelay(10);
//		}
	}
}
/* *************************************************************************
 * TaskHandle_t xBQTaskCreate(uint32_t taskpriority);
 * @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: BQTaskHandle
 * *************************************************************************/
TaskHandle_t xBQTaskCreate(uint32_t taskpriority)
{

/*
BaseType_t xTaskCreate( TaskFunction_t pvTaskCode,
const char * const pcName,
unsigned short usStackDepth,
void *pvParameters,
UBaseType_t uxPriority,
TaskHandle_t *pxCreatedTask );
*/
	BaseType_t ret = xTaskCreate(StartBQTask, "BQTask",\
     (128), NULL, taskpriority,\
     &BQTaskHandle);
	if (ret != pdPASS) return NULL;

	return BQTaskHandle;

}
/* *************************************************************************
 * static uint8_t getcellv(void);
 * @brief	: Read 16 cell voltages
 * *************************************************************************/
static uint8_t getcellv(void)
{
	int16_t* prcv = &cellv[(cvidx ^ 1)][0];
	uint32_t ret;

	/* Read 16: Cell voltages starting with Cell #1 at 0x14. */
	ret = readblock_cmd((uint8_t*)prcv, 16*2, 0x14);

	if (ret != 0) return (ret + 100);

	/* Read 4:  Stack Pack LD CC2 starting at 0x34. */
	prcv = &cellv[(cvidx ^ 1)][16];
	ret = readblock_cmd((uint8_t*)prcv, 4*2, 0x34);
	if (ret != 0) return (ret + 200);

	cvflag += 1;	// Show main a new reading

	return 0;


}
/* *************************************************************************
 * static void bqconfig(void);
 * @brief	: Configuration of BQ76952 upon startup of BQTask
 * *************************************************************************/
static uint8_t bqconfig(void)
{
	uint8_t ret;

 	ret = subcmdcom((uint8_t*)&device_number_u16, 2, DEVICE_NUMBER, 0x0, 0 );
 	if (ret != 0) return ret;

 	ret = subcmdcom((uint8_t*)&reg0_config_u16, 0, REG0Config, 0x0, 0 );
 	if (ret != 0) return ret;

 	ret = subcmdcom((uint8_t*)&ddsgp_config_u16, 1, DDSGPinConfig, 0x0, 0 );
 	if (ret != 0) return ret;
	return 0;
}
/* *************************************************************************
 * static uint8_t subcmdcom(uint8_t* pr, uint8_t nr, uint16_t cmd, uint16_t data, uint8_t nd);
 * @brief	: Execute a subcommand
 * @param   : pr = pointer to buffer to receive data 
 * @param   : nr = number of receive bytes to be read from BQ
 * @param   : cmd = subcommand code
 * @param   : data = subcommand data to be sent to BQ
 * @pararm  : nd = number of data bytes (0-2)
 * @return  : 0 = success;  > 0 is failure
 * *************************************************************************/
static uint8_t subcmdcom(uint8_t* pr, uint8_t nr, uint16_t cmd, uint16_t data, uint8_t nd)
{
	uint8_t ret;
	uint8_t bufchk[4];
	uint8_t bufx[6];
//	uint8_t A40 = 0x40; // Beginning of BQ transfer buffer memory address

	bufx[0] = 0x3E;     // Subcommand command code
	bufx[1] = cmd;		// Low ord sub command address
	bufx[2] = cmd >> 8; // Hi ord  sub command address
	bufx[3] = data;     // Low ord data
	bufx[4] = data >> 8; // Hi ord data

	/* Send  command. */
	ret = HAL_I2C_Master_Transmit(&hi2c1, 0x10, bufx, nd+3, 5000);
	if ( ret != HAL_OK ) return 1;

	if (nd > 0)
	{ // Here, command has one or two data bytes

		bufchk[0] = 0x60;  // Checksum address
		bufchk[1] = bqchksum(bufx,nd+3);
		bufchk[2] = nd+2; // Length for checksum
		osDelay(3);
		ret = HAL_I2C_Master_Transmit(&hi2c1, 0x10, bufchk, 3, 5000);
		if ( ret != HAL_OK ) return 4;
	}
 
   	if (nr > 0)
    { // Here, one or more received bytes expected 
   		/* Read results */
//   		osDelay(100);
 //  		ret = HAL_I2C_Master_Transmit(&hi2c1, 0x10, &A40, 1, 5000);
		osDelay(50);
		ret = HAL_I2C_Master_Receive(&hi2c1, 0x10, pr, nr, 5000);
   		if ( ret != HAL_OK ) return 3;
    }	
    return 0;
}

/* *************************************************************************
 * static uint8_t cmdcom(uint8_t* pr, uint8_t nr, uint8 cmd);
 * @brief	: Execute a subcommand
 * @param   : pr = pointer to buffer to receive data 
 * @param   : nr = number of receive bytes to be read from BQ
 * @param   : cmd = subcommand code
 * @return  : 0 = success;  > 0 is failure
 * *************************************************************************/
static uint8_t cmdcom(uint8_t* pr, uint8_t nr, uint8_t cmd)
{
	uint8_t ret;
	uint8_t cmdx = cmd;

	/* Send  command. */
	ret = HAL_I2C_Master_Transmit(&hi2c1, 0x10, &cmdx, 1, 5000);
	if ( ret != HAL_OK ) return 1;
 
   	if (nr > 0)
    { // Here, one or more bytes expected 
   		/* Read results */
		osDelay(20);
		ret |= HAL_I2C_Master_Receive(&hi2c1, 0x10, pr, nr, 5000);
   		if ( ret != HAL_OK ) return 3;
    }	
    return 0;
}

/* *************************************************************************
 * static void morse_sos(void);
 * @brief	: 
 * *************************************************************************/
static void morse_sos(void)
{
	morse_string("SOS",GPIO_PIN_1);
	osDelay(1000);

	return;
}
/* *************************************************************************
 * uint8_t bqchksum(uint8_t *ptr, uint8_t len);
 * @brief	: 
 * *************************************************************************/
uint8_t bqchksum(uint8_t *ptr, uint8_t len)
// Calculates the checksum when writing to a RAM register.
{
	uint8_t i;
	uint8_t checksum = 0;

	for(i=0; i<len; i++)
		checksum += ptr[i];

	checksum = 0xff & ~checksum;

	return (checksum);
}
/* *************************************************************************
 * static uint8_t readblock_cmd(uint8_t* pr, uint8_t nr, uint8_t cmd);
 * @brief	: Read block using direct commands
 * @param   : pr = pointer to buffer to receive bytes
 * @param   : nr = number of bytes to read
 * @param   : cmd = one byte "direct" command
 * *************************************************************************/
static uint8_t readblock_cmd(uint8_t* pr, uint8_t nr, uint8_t cmd)
{	
	uint8_t ret;
	uint8_t gcmd = cmd;

	/* Send "direct" command. */
	ret = HAL_I2C_Master_Transmit(&hi2c1, 0x10, &gcmd, 1, 5000);
   	if ( ret != HAL_OK ) 
   	{
   		bqflag = 11;
   	}
   	else
   	{ // Get readings
   		osDelay(3);
   		ret = HAL_I2C_Master_Receive(&hi2c1, 0x10, pr, nr, 5000);
   		if ( ret != HAL_OK ) 
   			bqflag = 13;
   		else
   			bqflag = 0;
   	}	
   	if (bqflag != 0)
   	{
   		*pr = 0xff; *(pr+1) = 0xff;
   	}
   	return ret;
 }