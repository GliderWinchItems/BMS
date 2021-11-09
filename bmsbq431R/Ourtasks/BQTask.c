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
#include "bq_idx_v_struct.h"
#include "bq_func_init.h"
#include "fetonoff.h"
#include "bq_items.h"

extern I2C_HandleTypeDef hi2c1;

static uint8_t bqconfig(void);
static uint8_t subcmdW(uint16_t cmd, uint16_t data, uint8_t nd, uint8_t config);
static uint8_t subcmdR(uint8_t* pr, uint8_t nr, uint16_t cmd);
static uint8_t dircmdR(uint8_t* pr, uint8_t nr, uint8_t cmd);
static uint8_t dircmdW(uint8_t* pr, uint8_t ns, uint8_t cmd);
static void morse_sos(void);
static uint8_t readblock_cmd(uint8_t* pr, uint8_t nr, uint8_t cmd);
static uint8_t getcellv(void);
static void read_all(void);
static void charger_update(struct BQFUNCTION* p);

extern osThreadId_t defaultTaskHandle;

struct BQFUNCTION bqfunction;

uint8_t bqchksum(uint8_t* ptr, uint8_t len);

TaskHandle_t BQTaskHandle = NULL;

 uint8_t bufcmd[4];
 uint8_t bufrcv[4];
 uint8_t bqflag = 0;
 //uint8_t bqbuf[34];

 int16_t cellv[2][BQVSIZE];
 uint8_t cvidx = 0;
 uint32_t cvflag = 0;

uint16_t alarm_status[3]; // 0x62 TRM:105
uint16_t alarm_status_prev;
uint16_t alarm_status_ctr;
uint16_t alarm_status_ctr1;
uint16_t alarm_reset = 0x0082; //0x0002;


/* Readings */ 
uint16_t device_number_u16;
uint16_t fw_version_u16[3];
uint16_t hw_version_u16;
uint16_t battery_status;
uint16_t reg0_config_u16;
uint16_t ddsgp_config_u16;
uint16_t blk_0x62_u16[14];

/* ADC counts for voltage and sync'd current */
union ADCCELLCOUNTS cellcts;


int16_t blk_0x0075_s16[16];

/* Cell balancing */
uint16_t blk_0x0083_u16[3];
int16_t cuv_snap_0x0080_s16[16];
int16_t cov_snap_0x0081_s16[16];
uint32_t cb_status2_0x0086_u32[8]; // Seconds active cell balancing: 1 - 8
uint32_t cb_status3_0x0087_u32[8]; // Seconds active cell balancing: 9 - 16


uint8_t fw_version_6[6];
uint8_t blk_0x9231_12[12];
uint8_t blk_0x92fa_11[11];
uint8_t blk_0x9335_14[14];

uint8_t bq_initflag = 0;

/* *************************************************************************
 * static void brake_init(struct BQFUNCTION* p);
 *	@brief	: Some initialization before endless loop starts
 * *************************************************************************/
static void bq_init(void)
{
		uint8_t ret;

		/* ENTER configuration mode. */
		ret = subcmdW(SET_CFGUPDATE, 0x0, 0, 0);
		if (ret != 0) morse_string("HI",GPIO_PIN_1);

		/* Enable REG0. */
		//       (uint16_t cmd, uint16_t data, uint8_t nd, uint8_t config);
		ret = subcmdW(REG0Config, 0x1, 1, 0);
		if (ret != 0) morse_string("S",GPIO_PIN_1);

		/* Enable REG12. REG0: 2.5v, REG1: not enabled */
		//       (uint16_t cmd, uint16_t data, uint8_t nd, uint8_t config);
		ret = subcmdW(REG12Config, 0x09, 1, 0);
		if (ret != 0) morse_string("S",GPIO_PIN_1);

		/* Set no communications shutdown duration: minutes. */
		//       (uint16_t cmd, uint16_t data, uint8_t nd, uint8_t config);
		ret = subcmdW(CommIdleTime, 0x1, 1, 0);
		if (ret != 0) morse_string("H",GPIO_PIN_1);

		/* Max number of cell fets on at same time. */
		ret = subcmdW(CellBalanceMaxCells, 0x7, 1, 0);
		if (ret != 0) morse_string("H",GPIO_PIN_1);

		/* Slow loop mode when balancing. */
		ret = subcmdW(PowerConfig,(uint16_t)0x29BE, 1, 0);
		if (ret != 0) morse_string("H",GPIO_PIN_1);

		/* Alert inputs to L431. Active low; Alert mode. */
		ret = subcmdW(ALERTPinConfig,   0x82, 1, 0); 
		if (ret != 0) morse_string("N",GPIO_PIN_1);		

		/* Alarm enable: measurement scan complete */
		ret = subcmdW(AlarmEnable,   0x02, 1, 0); 
		if (ret != 0) morse_string("AM",GPIO_PIN_1);	

		/* Alarm enable: loads upon reset of exit config */
		ret = subcmdW(DefaultAlarmMask, 0x82, 2, 0); // Fullscan, Scan complete
		if (ret != 0) morse_string("AD",GPIO_PIN_1);

		/* Cell balance stop delta. (mv) */
		ret = subcmdW(CellBalanceMinCellVCharge,(uint16_t)bqfunction.lc.cellv_max, 2, 0); 
		if (ret != 0) morse_string("CBX",GPIO_PIN_1);


		/* Cell balance stop delta. (mv) */
		ret = subcmdW(CellBalanceStopDeltaCharge, 5, 1, 0); 
		if (ret != 0) morse_string("CBX",GPIO_PIN_1);

		/* Cell balance minimum (mv) */
		ret = subcmdW(CellBalanceMinDeltaCharge, 10, 1, 0); 
		if (ret != 0) morse_string("CBY",GPIO_PIN_1);

		/* EXIT configuration mode. */
		ret = subcmdW(EXIT_CFGUPDATE, 0x0, 0, 0);
		if (ret != 0) morse_string("HI",GPIO_PIN_1);

/* Thermistor mode. */
				//       (uint16_t cmd, uint16_t data, uint8_t nd, uint8_t config);
		ret = subcmdW(CFETOFFPinConfig,  0x2, 1, 0); // Therm 5
		if (ret != 0) morse_string("A",GPIO_PIN_1);
		ret = subcmdW(DFETOFFPinConfig,  0x2, 1, 0); // Therm 4
		if (ret != 0) morse_string("B",GPIO_PIN_1);
		ret = subcmdW(TS1Config,         0x2, 1, 0); // Therm 1
		if (ret != 0) morse_string("D",GPIO_PIN_1);
		ret = subcmdW(TS3Config,         0x2, 1, 0); // Therm 2
		if (ret != 0) morse_string("B",GPIO_PIN_1);
		ret = subcmdW(DCHGPinConfig,     0x2, 1, 0); // Therm 3
		if (ret != 0) morse_string("U",GPIO_PIN_1);

		/* GPIO. */
// $$$$ 0x09 uses REG18 for GPIO voltage active high. Just for test		
		ret = subcmdW(DDSGPinConfig,    0x09, 1, 1); // REG18 pullup, active hi
		if (ret != 0) morse_string("U",GPIO_PIN_1);

		/* Read battery status byte. */
		ret = dircmdR((uint8_t *)&battery_status, 2, BatteryStatus);
		if (ret != 0) morse_string("E",GPIO_PIN_1);

		/* Read device number. */
		//                       (uint8_t* pr, uint8_t nr, uint16_t cmd)
		ret = subcmdR((uint8_t*)&device_number_u16, 2, DEVICE_NUMBER);
		if (ret != 0) morse_string("I",GPIO_PIN_1);

		// Device, Firmware
		ret = subcmdR((uint8_t*)&fw_version_6, 6, FW_VERSION);
		if (ret != 0) morse_string("I",GPIO_PIN_1);
		// Convert to Big Endian
		fw_version_u16[0] = fw_version_6[0] << 8 | fw_version_6[1];
		fw_version_u16[1] = fw_version_6[2] << 8 | fw_version_6[3];
		fw_version_u16[2] = fw_version_6[4] << 8 | fw_version_6[5];

		ret = subcmdR((uint8_t*)&hw_version_u16, 2, HW_VERSION);
		if (ret != 0) morse_string("I",GPIO_PIN_1);

	bq_initflag = 1; // Show init has been completed
	
	return;
}
/* *************************************************************************
 * void StartBQTask(void const * argument);
 *	@brief	: Task startup
 * *************************************************************************/
uint8_t dbgx; // Debug

void StartBQTask(void* argument)
{
while(1==1) osDelay(100); // Temp nix BQ'ing
//	struct BQFUNCTION* p;
//	int i;

//#define DDSG_TEST
#ifdef DDSG_TEST	
uint8_t ddsgalt = 0;
uint8_t ddsgctr = 0;
uint8_t bqstate = 0;
#endif


	struct BQFUNCTION* pbq = &bqfunction;
	uint8_t ret;            // Error check return
	uint8_t bqstate = 0;    // State machine
	uint8_t tmp;
	uint8_t i;

	TickType_t xLastWakeTime;

	/* Wake up BQ if it is SHUTDOWN mode. */
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);
	osDelay(300); // Wait for BQ to initialize/reset
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_SET);
	osDelay(300);
	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,GPIO_PIN_RESET);

	bq_init();

	ret = bqconfig();
	if (ret != 0)
	{
		morse_sos();
		osDelay(50);
	}

	xLastWakeTime = xTaskGetTickCount();

	for (;;)
	{
		osDelay(10); // Polling loop; Later have ALERT interrupt & notify
		/* I think this is needed if ribbon cabled pulled and re-plugged. */
		if (bq_initflag == 0) 
			bq_init();

#ifdef USEALARMSTATUSSTUFF
		/* Read: ALARMstatus 0x62, AlarmRawStatus 0x64, AlarmEnable 0x66  */
		ret = dircmdR((uint8_t *)&alarm_status[0], 3*2, AlarmStatus);
		if (ret != 0) morse_string("AL",GPIO_PIN_1);
		if ((alarm_status[0] & 0x2) != alarm_status_prev) 
		{
			alarm_status_prev = (alarm_status[0] & 0x2);
			alarm_status_ctr += 1;
			osDelay(3);
			ret = dircmdW((uint8_t*)&alarm_reset, 2, AlarmStatus); // 0x62: Write 1 to clear bit
			if (ret != 0) morse_string("AW",GPIO_PIN_1);
		}
#endif
//		osDelay(3);
//		ret = dircmdR((uint8_t *)&alarm_status[0], 3*2, AlarmStatus);

	osDelay(3);
		switch (bqstate)
		{
		case 0: // Balancing loop

//			ret = subcmdW(CB_ACTIVE_CELLS,(uint16_t)0x0000, 2, 0); // Active cells balancing
//			if (ret != 0) morse_string("BA",GPIO_PIN_1);

			/* Turn OFF all fets: charger(s), heater, etc. */
			fetonoff_status_set(0);
fetonoff(FET_DUMP,0);
			/* Turn off any dishcarging. */
			ret = subcmdW(CB_ACTIVE_CELLS,(uint16_t)0x0, 2, 0); 
			if (ret != 0) morse_string("BA",GPIO_PIN_1);

			/* Read everything */
			read_all(); // This takes ~(100 * 13) ms

			/* Get cell voltages (with charging/discharging off). */
			ret = getcellv();
			if (ret != 0) morse_string("GET1",GPIO_PIN_1);

			/* Determine cells to balance. */
			bq_items_seq(&cellv[(cvidx)][0]);

			/* Set FETs accordingly. */
			fetonoff_status_set(pbq->fet_status);

			/* Set cell balancing fets.  */
			// uint8_t subcmdW(uint16_t cmd, uint16_t data, uint8_t nd, uint8_t config)
			ret = subcmdW(CB_ACTIVE_CELLS,(uint16_t)pbq->cellbal, 2, 0); // Active cells balancing
			if (ret != 0) morse_string("BA",GPIO_PIN_1);
osDelay(3);
			/* Cell balancing data. */
			ret = subcmdR((uint8_t*)&blk_0x0083_u16[0], 3*2, 0x0083);
			if (ret != 0) morse_string("C3",GPIO_PIN_1);	
osDelay(3);
			/* Read cell balancing parameters. */
			ret = subcmdR((uint8_t*)&blk_0x9335_14, 14, BalancingConfiguration);
			if (ret != 0) morse_string("CB3",GPIO_PIN_1);
osDelay(3);
			ret = subcmdW(CB_ACTIVE_CELLS,(uint16_t)pbq->cellbal, 2, 0); // Active cells balancing
			if (ret != 0) morse_string("BA",GPIO_PIN_1);

			/* Update charging/discharging */
			charger_update(pbq);	

			/* Trigger 'main.c' to display all the crap retrieved. */
			xTaskNotify(defaultTaskHandle, DEFAULTTASKBIT01, eSetBits);

			/* Delay loop. Approx 10 sec per loop. */
			vTaskDelayUntil( &xLastWakeTime,pdMS_TO_TICKS(10000) );
			break;

		case 1:	// DUMP discharge until cutoff voltage. 
			/* Turn off any FET dishcarging. */
			ret = subcmdW(CB_ACTIVE_CELLS,(uint16_t)0x0, 2, 0); 
			if (ret != 0) morse_string("BA",GPIO_PIN_1);

			/* Read everything */
			read_all(); // This takes ~(100 * 13) ms

			/* Turn OFF all fets: charger(s), heater, etc. */
			fetonoff_status_set(0);

			/* Get cell voltages (with charging/discharging off). */
			ret = getcellv();
			if (ret != 0) morse_string("GET2",GPIO_PIN_1);

			/* Check for any cell at cutoff voltage. */
			int16_t* p = &cellv[(cvidx)][0];
			tmp = 0;
			for (i = 0; i < pbq->lc.ncell; i++)
			{
				if (*p++ < pbq->lc.cellv_min) tmp = 1;
			}
			if (tmp != 0)
			{ // Here, stop discharging
				/* Turn OFF all fets: charger(s), heater, etc. */
				fetonoff_status_set(0);
			}
			else
			{ // Here, start discharging
				fetonoff(FET_DUMP, FET_SETON);
			}

			/* Trigger 'main.c' to display all the crap retrieved. */
			xTaskNotify(defaultTaskHandle, DEFAULTTASKBIT01, eSetBits);

			/* Fixed delay for loop. */
			vTaskDelayUntil( &xLastWakeTime,pdMS_TO_TICKS(3000) );

			break;	
		}
	}

// Save code for now
#ifdef DDSG_TEST // Test DDSG gpio on/off working
ddsgctr += 1;
if (ddsgctr >= 4)	
{
	ddsgctr = 0;
	if ((ddsgalt & 1) == 0)
		subcmdW(DDSG_HI, 0x0, 0, 0);
	else
		subcmdW(DDSG_LO, 0x0, 0, 0);
	ddsgalt ^= 1;
}
#endif

}
/* *************************************************************************
 * TaskHandle_t xBQTaskCreate(uint32_t taskpriority);
 * @brief	: Create task; task handle created is global for all to enjoy!
 * @param	: taskpriority = Task priority (just as it says!)
 * @return	: BQTaskHandle
 * *************************************************************************/
TaskHandle_t xBQTaskCreate(uint32_t taskpriority)
{
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
	int16_t* prcv = &cellv[(cvidx)][0];
	uint32_t ret;

	/* Read 16: Cell voltages starting with Cell #1 at 0x14. */
	ret = readblock_cmd((uint8_t*)prcv, 16*2, 0x14);
	if (ret != 0) return (ret + 100);

	/* Read 4:  Stack Pack LD CC2 starting at 0x34. */
	prcv = &cellv[(cvidx)][16];
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

 		/* Read battery status byte. */
		ret = dircmdR((uint8_t *)&battery_status, 2, BatteryStatus);
		if (ret != 0) morse_string("E",GPIO_PIN_1);

		/* Read device number. */
		//                       (uint8_t* pr, uint8_t nr, uint16_t cmd)
		ret = subcmdR((uint8_t*)&device_number_u16, 2, DEVICE_NUMBER);
		if (ret != 0) morse_string("I",GPIO_PIN_1);

		/* Enable REG0. */
		//       (uint16_t cmd, uint16_t data, uint8_t nd, uint8_t config);
		ret = subcmdW(REG0Config, 0x1, 1, 1);
		if (ret != 0) morse_string("S",GPIO_PIN_1);

	return 0;
}
/* *************************************************************************
 * static uint8_t subcmdW(uint16_t cmd, uint16_t data, uint8_t nd, uint8_t config);
 * @brief	: Execute a subcommand that writes and/or requires going into confiure mode.
 * @param   : cmd = subcommand code
 * @param   : data = subcommand data to be sent to BQ (two bytes little Endian in uin16_t)
 * @pararm  : nd = number of data bytes (0-2)
 * @param   : config = 1, enter/exit UpdateConfig mode
 * @return  : 0 = success;  > 0 is failure
 * *************************************************************************/
uint8_t dbA[16];
static uint8_t subcmdW(uint16_t cmd, uint16_t data, uint8_t nd, uint8_t config)
{
	uint8_t ret;
	uint8_t bufchk[4];
	uint8_t bufx[6];
	uint8_t bufc[3]; // Configuration

	bufx[0] = 0x3E;     // Subcommand command code
	bufx[1] = cmd;		// Low ord sub command address
	bufx[2] = cmd >> 8; // Hi ord  sub command address
	bufx[3] = data;     // Low ord data
	bufx[4] = data >> 8; // Hi ord data (if used)

	/* Enter Update_Config mode, if requested. */
	if (config == 1)
	{ // Update requested
		bufc[0] = 0x3E;
		bufc[1] = SET_CFGUPDATE;
		bufc[2] = SET_CFGUPDATE >> 8;
		ret = HAL_I2C_Master_Transmit(&hi2c1, 0x10, bufc, 3, 5000);
		if ( ret != HAL_OK ) return 31;
		osDelay(10);
	}

	/* Send  command. */
	ret = HAL_I2C_Master_Transmit(&hi2c1, 0x10, bufx, nd+3, 5000);
	if ( ret != HAL_OK ) return 32;
dbA[0]=bufx[0];	
dbA[1]=bufx[1];	
dbA[2]=bufx[2];
dbA[3]=bufx[3];

	if (nd > 0)
	{ // Here, command has one or two data bytes

		bufchk[0] = 0x60;  // Checksum address
		bufchk[1] = bqchksum(&bufx[1],nd+2);
		bufchk[2] = nd+4; // Length for checksum
dbA[4]=bufchk[0];	
dbA[5]=bufchk[1];	
dbA[6]=bufchk[2];	
		osDelay(3);
		ret = HAL_I2C_Master_Transmit(&hi2c1, 0x10, bufchk, 3, 5000);
		if ( ret != HAL_OK ) return 33;
		osDelay(300);
	}

	if (config == 1)
	{ // Update requested
		bufc[0] = 0x3E;
		bufc[1] = EXIT_CFGUPDATE;
		bufc[2] = EXIT_CFGUPDATE >> 8;
		ret = HAL_I2C_Master_Transmit(&hi2c1, 0x10, bufc, 3, 5000);
		if ( ret != HAL_OK ) return 31;
		osDelay(10);
	}

	osDelay(3);
 
    return 0;
}
/* *************************************************************************
 * static uint8_t subcmdR(uint8_t* pr, uint8_t nr, uint16_t cmd);
 * @brief	: Execute a subcommand that reads
 * @param   : pr = pointer to buffer to receive data 
 * @param   : nr = number of receive bytes to be read from BQ (0-32)
 * @param   : cmd = subcommand code
 * @return  : 0 = success;  > 0 is failure
 * *************************************************************************/
static uint8_t subcmdR(uint8_t* pr, uint8_t nr, uint16_t cmd)
{
	uint8_t ret;
	uint8_t bufx[6];
	uint8_t A40 = 0x40; // Beginning of BQ transfer buffer memory address

	bufx[0] = 0x3E;     // Subcommand command code
	bufx[1] = cmd;		// Low ord sub command address
	bufx[2] = cmd >> 8; // Hi ord  sub command address

	/* Send  subcommand. */
	ret = HAL_I2C_Master_Transmit(&hi2c1, 0x10, bufx, 3, 5000);
	if ( ret != HAL_OK ) return 21;
	osDelay(3);

   	if (nr > 0)
    { // Here, one or more received bytes expected 
   		/* Read results */
   		osDelay(100);
  		ret = HAL_I2C_Master_Transmit(&hi2c1, 0x10, &A40, 1, 5000);
		osDelay(5);
		ret = HAL_I2C_Master_Receive(&hi2c1, 0x10, pr, nr, 5000);
   		if ( ret != HAL_OK ) return 22;
   		osDelay(3);
    }	
    return 0;
}
/* *************************************************************************
 * static uint8_t dircmdR(uint8_t* pr, uint8_t nr, uint8 cmd);
 * @brief	: Execute a one byte command
 * @param   : pr = pointer to buffer to receive data 
 * @param   : nr = number of receive bytes to be read from BQ
 * @param   : cmd = one byte command code
 * @return  : 0 = success;  > 0 is failure
 * *************************************************************************/
static uint8_t dircmdR(uint8_t* pr, uint8_t nr, uint8_t cmd)
{
	uint8_t ret;
	uint8_t cmdx = cmd;

	/* Send  command. */
	ret = HAL_I2C_Master_Transmit(&hi2c1, 0x10, &cmdx, 1, 5000);
	if ( ret != HAL_OK ) return 1;
 
   	if (nr > 0)
    { // Here, one or more bytes expected 
   		/* Read results */
		osDelay(10);
		ret |= HAL_I2C_Master_Receive(&hi2c1, 0x10, pr, nr, 5000);
   		if ( ret != HAL_OK ) return 3;
    }	
    return 0;
}
/* *************************************************************************
 * static uint8_t dircmdW(uint8_t* pr, uint8_t ns, uint8_t cmd);
 * @brief	: Execute a one byte command that sends data
 * @param   : pr = pointer to buffer with data to be sent
 * @param   : nw = number of bytes to send
 * @param   : cmd = one byte command code
 * @return  : 0 = success;  > 0 is failure
 * *************************************************************************/
static uint8_t dircmdW(uint8_t* pr, uint8_t ns, uint8_t cmd)
{
	uint8_t ret;
	uint8_t cmdx[4];
	cmdx[0] = cmd;
	cmdx[1] = *(pr+0);
	cmdx[2] = *(pr+1);

	/* Send  command. */
	ret = HAL_I2C_Master_Transmit(&hi2c1, 0x10, &cmdx[0], (ns+1), 5000);
	if ( ret != HAL_OK ) return 1;
 
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
   	osDelay(3);
   	return ret;
 }
 /* *************************************************************************
 * static void read_all(void);
 * @brief	: Read bq data
 * *************************************************************************/
static void read_all(void)
{
		int ret;

 		/* Read block with lots of stuff. */
		ret = subcmdR((uint8_t*)&blk_0x9231_12, 12, 0x9231);
		if (ret != 0) morse_string("T",GPIO_PIN_1);	

		/* Read block with lots of stuff. */
		ret = subcmdR((uint8_t*)&blk_0x92fa_11, 11, 0x92fa);
		if (ret != 0) morse_string("M",GPIO_PIN_1);	

		
		ret = readblock_cmd((uint8_t*)&blk_0x62_u16[0], 14*2, 0x62);
		if (ret != 0) morse_string("W",GPIO_PIN_1);	

		/* Cell balancing data. */
		ret = subcmdR((uint8_t*)&blk_0x0083_u16[0], 3*2, 0x0083);
		if (ret != 0) morse_string("C3",GPIO_PIN_1);	

		// Records the cell voltage measurement made just after the latest CUV event.
		ret = subcmdR((uint8_t*)&cuv_snap_0x0080_s16[0], 16*2, 0x0080);
		if (ret != 0) morse_string("CSP1",GPIO_PIN_1);

		// Records the cell voltage measurement made just after the latest COV event
		ret = subcmdR((uint8_t*)&cov_snap_0x0081_s16[0], 16*2, 0x0081);
		if (ret != 0) morse_string("CSP2",GPIO_PIN_1);	

		// Reports the cumulative number of seconds that balancing has been 
		//   active on this cell since the last device reset.
		ret = subcmdR((uint8_t*)&cb_status2_0x0086_u32[0], 8*4, 0x0086);
		if (ret != 0) morse_string("CBT1",GPIO_PIN_1);	
		ret = subcmdR((uint8_t*)&cb_status3_0x0087_u32[0], 8*4, 0x0087);
		if (ret != 0) morse_string("CBT2",GPIO_PIN_1);	

		/* Read ADC counts for current and voltage for each cell. */
		// ADC count for current measurement taken during the cell voltage measurement.
		ret = subcmdR((uint8_t*)&cellcts.blk[0][0], 4*2*4, 0x0071);
		if (ret != 0) morse_string("DS1",GPIO_PIN_1);	
		ret = subcmdR((uint8_t*)&cellcts.blk[4][0], 4*2*4, 0x0072);
		if (ret != 0) morse_string("DS1",GPIO_PIN_1);	
		ret = subcmdR((uint8_t*)&cellcts.blk[8][0], 4*2*4, 0x0073);
		if (ret != 0) morse_string("DS1",GPIO_PIN_1);	
		ret = subcmdR((uint8_t*)&cellcts.blk[12][0], 4*2*4, 0x0074);
		if (ret != 0) morse_string("DS1",GPIO_PIN_1);	

		/* Misc ADC counts, VREG18 - CC3. */
		ret = subcmdR((uint8_t*)&blk_0x0075_s16[0], 16*2, 0x0075);
		if (ret != 0) morse_string("DS5",GPIO_PIN_1);
	return;
}	
/* *************************************************************************
 * static void charger_update(struct BQFUNCTION* p);
 * @brief	: Update charger/discharging FETs based on status byte
 * *************************************************************************/
static void charger_update(struct BQFUNCTION* p)
{
	/* Internal charger control. */
	if ((p->fet_status & FET_CHGR) != 0)
	{ // Here, set charging 
		TIM1->CCR1 = bqfunction.tim1_ccr1; // Set charge rate
	}
	else
	{ // If not normal rate, should it be Very Low Charge rate?
		if ((p->fet_status & FET_CHGR_VLC) != 0)
		{ // Here, yes.
			TIM1->CCR1 = bqfunction.lc.tim1_ccr1_on_vlc; // Set vlc rate
		}
		else
		{ // Here, stop charging
			
		}
	}
//		
//p->fet_status &= ~FET_CHGR;
#ifdef USETHISFETSTUFF
	/* DUMP controls module discharging FET. */
	if ((p->fet_status & FET_DUMP) != 0)
	{ // Here, turn on FET for resistor discharge
		fetonoff(FET_DUMP,  FET_SETON);
	}
	else
	{ // Here, turn off FET for resistor discharge
		fetonoff(FET_DUMP,  FET_SETOFF);
	}

	/* DUMP2 controls external charger. */
	if ((p->fet_status & FET_DUMP2) != 0)
	{ // Here, turn on external module (or string?) charger
		fetonoff(FET_DUMP2,  FET_SETON);
	}
	else
	{ // Here, turn it off.
		fetonoff(FET_DUMP2,  FET_SETOFF);
	}
#endif

	return;
}
