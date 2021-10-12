/******************************************************************************
* File Name          : bqview.c
* Date First Issued  : 09/18/2021
* Description        : BQ76952: yprintf BQ data
*******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "malloc.h"

#include "main.h"
#include "morse.h"

#include "BQTask.h"
#include "bqview.h"
#include "bqcellbal.h"
#include "yprintf.h"
#include "BQ769x2Header.h"


/* *************************************************************************
 * void bqview_blk_0x9231 (struct SERIALSENDTASKBCB** pp);
 *	@brief	: display parameters
 * *************************************************************************/
extern uint8_t blk_0x9231_12[12];
void bqview_blk_0x9231 (struct SERIALSENDTASKBCB** pp)
{
yprintf(pp,"\n\rMinBlowFuseVoltage  0x9231 %5d mv",10*( (uint32_t)blk_0x9231_12[0] | (blk_0x9231_12[1] << 8) ));//Settings:Fuse:Min Blow Fuse Voltage (10 mv)
yprintf(pp,"\n\rFuseBlowTimeout     0x9233 %5d sec", blk_0x9231_12[ 2]);     //Settings:Fuse:Fuse Blow Timeout			
yprintf(pp,"\n\rPowerConfig         0x9234 0x%04X", (uint16_t)(blk_0x9231_12[ 3] | blk_0x9231_12[ 4]<<8 ));//Settings:Configuration:Power Config			
yprintf(pp,"\n\rREG12Config         0x9236 0x%02X", blk_0x9231_12[ 5]);      //Settings:Configuration:REG12 Config			
yprintf(pp,"\n\rREG0Config          0x9237 0x%02X", blk_0x9231_12[ 6]);      //Settings:Configuration:REG0 Config			
yprintf(pp,"\n\rHWDRegulatorOptions 0x9238 0x%02X", blk_0x9231_12[ 7]);      //Settings:Configuration:HWD Regulator Options			
yprintf(pp,"\n\rCommType            0x9239 0x%02X", blk_0x9231_12[ 8]);      //Settings:Configuration:Comm Type			
yprintf(pp,"\n\rI2CAddress          0x923A 0x%02X", blk_0x9231_12[ 9]);      //Settings:Configuration:I2C Address			
yprintf(pp,"\n\rSPIConfiguration    0x923C 0x%02X", blk_0x9231_12[10]);      //Settings:Configuration:SPI Configuration			
yprintf(pp,"\n\rCommIdleTime        0x923D 0x%02X", blk_0x9231_12[11]);      //Settings:Configuration:Comm Idle Time			
	return;
}

/* *************************************************************************
 * void bqview_blk_0x92fa (struct SERIALSENDTASKBCB** pp);
 *	@brief	: display parameters
 * *************************************************************************/
extern uint8_t blk_0x92fa_11[11];
void bqview_blk_0x92fa (struct SERIALSENDTASKBCB** pp)
{
yprintf(pp,"\n\rCFETOFFPinConfig 0x92FA 0x%02X",blk_0x92fa_11[ 0]);       //Settings:Configuration:CFETOFF Pin Config			
yprintf(pp,"\n\rDFETOFFPinConfig 0x92FB 0x%02X",blk_0x92fa_11[ 1]);       //Settings:Configuration:DFETOFF Pin Config			
yprintf(pp,"\n\rALERTPinConfig   0x92FC 0x%02X",blk_0x92fa_11[ 2]);       //Settings:Configuration:ALERT Pin Config			
yprintf(pp,"\n\rTS1Config        0x92FD 0x%02X",blk_0x92fa_11[ 3]);       //Settings:Configuration:TS1 Config			
yprintf(pp,"\n\rTS2Config        0x92FE 0x%02X",blk_0x92fa_11[ 4]);       //Settings:Configuration:TS2 Config			
yprintf(pp,"\n\rTS3Config        0x92FF 0x%02X",blk_0x92fa_11[ 5]);       //Settings:Configuration:TS3 Config			
yprintf(pp,"\n\rHDQPinConfig     0x9300 0x%02X",blk_0x92fa_11[ 6]);       //Settings:Configuration:HDQ Pin Config			
yprintf(pp,"\n\rDCHGPinConfig    0x9301 0x%02X",blk_0x92fa_11[ 7]);       //Settings:Configuration:DCHG Pin Config			
yprintf(pp,"\n\rDDSGPinConfig    0x9302 0x%02X",blk_0x92fa_11[ 8]);       //Settings:Configuration:DDSG Pin Config			
yprintf(pp,"\n\rDAConfiguration  0x9303 0x%02X",blk_0x92fa_11[ 9]);       //Settings:Configuration:DA Configuration			
yprintf(pp,"\n\rVCellMode        0x9304 0x%02X",blk_0x92fa_11[10]);       //Settings:Configuration:Vcell Mode			
yprintf(pp,"\n\rCC3Samples       0x9307 0x%02X",blk_0x92fa_11[11]);       //Settings:Configuration:CC3 Samples		
		return;

}

/* *************************************************************************
 * void bqview_blk_0x62 (struct SERIALSENDTASKBCB** pp);
 *	@brief	: display parameters
 * *************************************************************************/
extern uint16_t blk_0x62_u16[14];
void bqview_blk_0x62 (struct SERIALSENDTASKBCB** pp)
{
yprintf(pp,"\n\rAlarmStatus        0x62 0x%04X",blk_0x62_u16[ 0]); // Alarm Status: Latched signal used to assert the ALERT pin
yprintf(pp,"\n\rAlarmRawStatus     0x64 0x%04X",blk_0x62_u16[ 1]); // Alarm Raw Status: Unlatched value of flags
yprintf(pp,"\n\rAlarmEnable        0x66 0x%04X",blk_0x62_u16[ 2]); // Alarm Enable:Mask for Alarm Status
yprintf(pp,"\n\rIntTemperature     0x68   %5d",(int16_t)blk_0x62_u16[ 3]-2732); // Int Temperature: most recent measured internal die temperature
yprintf(pp,"\n\rCFETOFFTemperature 0x6A   %5d",(int16_t)blk_0x62_u16[ 4]     ); // Config: thermistor-most recent temp (0.1K); ADCIN-reading in mv
yprintf(pp,"\n\rDFETOFFTemperature 0x6C   %5d",(int16_t)blk_0x62_u16[ 5]     ); // Config: thermistor-most recent temp (0.1K); ADCIN-reading in mv 
yprintf(pp,"\n\rALERTTemperature   0x6E   %5d",(int16_t)blk_0x62_u16[ 6]     ); // Config: thermistor-most recent temp (0.1K); ADCIN-reading in mv
yprintf(pp,"\n\rTS1Temperature     0x70   %5d",(int16_t)blk_0x62_u16[ 7]     ); // Config: thermistor-most recent temp (0.1K); ADCIN-reading in mv
yprintf(pp,"\n\rTS2Temperature     0x72   %5d",(int16_t)blk_0x62_u16[ 8]     ); // Config: thermistor-most recent temp (0.1K); ADCIN-reading in mv
yprintf(pp,"\n\rTS3Temperature     0x74   %5d",(int16_t)blk_0x62_u16[ 9]     ); // Config: thermistor-most recent temp (0.1K); ADCIN-reading in mv
yprintf(pp,"\n\rHDQTemperature     0x76   %5d",(int16_t)blk_0x62_u16[10]     ); // Config: thermistor-most recent temp (0.1K); ADCIN-reading in mv
yprintf(pp,"\n\rDCHGTemperature    0x78   %5d",(int16_t)blk_0x62_u16[11]     ); // Config: thermistor-most recent temp (0.1K); ADCIN-reading in mv
yprintf(pp,"\n\rDDSGTemperature    0x7A   %5d",(int16_t)blk_0x62_u16[12]     ); // Config: thermistor-most recent temp (0.1K); ADCIN-reading in mv
yprintf(pp,"\n\rFETStatus          0x7F 0x%02X",blk_0x62_u16[13]); // FET Status: flags showing status of FETs and ALERT pin
	return;
}
/* *************************************************************************
 * void bqview_blk_0x0083 (struct SERIALSENDTASKBCB** pp);
 *	@brief	: display parameters
 * *************************************************************************/
extern uint16_t blk_0x0083_u16[3];
void bqview_blk_0x0083 (struct SERIALSENDTASKBCB** pp)
{
	int i;
	char c;
	uint16_t tmp = blk_0x0083_u16[0];

// Cells actively being balanced
//	yprintf(pp,"\n\rCB_ACTIVE_CELLS 0x0083");
//	for (i = 1; i < 17; i++) yprintf(pp,"%4d", i); // Cell number header
	yprintf(pp,"\n\r        ");
	for (i = 0; i < 16; i++)
	{
		if ((tmp & (1 << i)) == 0) 
			c = '.';
		else
			c = '#';
		yprintf(pp,"       %c", c);
	}

		return;
}
/* *************************************************************************
 * void bqview_cuv_cov_snap_0x0080_0x0081 (struct SERIALSENDTASKBCB** pp);
 *	@brief	: display parameters
 * *************************************************************************/
extern int16_t cuv_snap_0x0080_s16[16];
extern int16_t cov_snap_0x0081_s16[16];
void bqview_cuv_cov_snap_0x0080_0x0081 (struct SERIALSENDTASKBCB** pp)
{
	int i;
	yprintf(pp,"\n\rCUV_SNAPSHOT 0x0080 COV_SNAPSHOT 0X0081 (Under a& Over voltage snapshot)\n\r  ");
	for (i = 1; i < 17; i++) yprintf(pp," %5d", i);
	yprintf(pp,"\n\r  ");

	for (i = 0; i < 16; i++)
		yprintf(pp,"%6d",(int16_t)cuv_snap_0x0080_s16[i]);

	yprintf(pp,"\n\r  ");
	for (i = 0; i < 16; i++)
		yprintf(pp,"%6d",(int16_t)cov_snap_0x0081_s16[i]);
	return;
}
/* *************************************************************************
 * void bqview_cb_status2_0x0086_0x0087 (struct SERIALSENDTASKBCB** pp);
 *	@brief	: display parameters
 * *************************************************************************/
extern uint32_t cb_status2_0x0086_u32[8]; // Seconds active cell balancing: 1 - 9
extern uint32_t cb_status3_0x0087_u32[8]; // Seconds active cell balancing: 9 - 16
void bqview_cb_status2_0x0086_0x0087 (struct SERIALSENDTASKBCB** pp)
{
	int i;
	yprintf(pp,"\n\rCBSTATUS2 -CBSTATUS3 (0x0086 - 0x0087) (Total balancing time (secs) )\n\r        ");
	for (i = 1; i < 17; i++) yprintf(pp,"%8d", i);
	yprintf(pp,"\n\r        ");

	for (i = 0; i < 8; i++)
		yprintf(pp,"%8d",(uint32_t)cb_status2_0x0086_u32[i]);

	for (i = 0; i < 8; i++)
		yprintf(pp,"%8d",(uint32_t)cb_status3_0x0087_u32[i]);
	return;

}
/* *************************************************************************
 * void bqview_blk_0x0075_s16 (struct SERIALSENDTASKBCB** pp);
 *	@brief	: display parameters
 * *************************************************************************/
extern int16_t blk_0x0075_s16[16];
void bqview_blk_0x0075_s16 (struct SERIALSENDTASKBCB** pp)
{
yprintf(pp,"\n\rDASTATUS5 0x0075");
yprintf(pp,"\n\r\t  0 VREG18 %5d adc_ct",blk_0x0075_s16[0]);
yprintf(pp,"\n\r\t  2 VSS    %5d adc_ct",blk_0x0075_s16[1]);
yprintf(pp,"\n\r\t  4 Max Cell Voltage %5d mv",blk_0x0075_s16[2]);
yprintf(pp,"\n\r\t  6 Min Cell Voltage %5d mv",blk_0x0075_s16[3]);
yprintf(pp,"\n\r\t  8 Battery sum      %5d userV",blk_0x0075_s16[4]);
yprintf(pp,"\n\r\t 10 Cell temperature %5d 0.1K",blk_0x0075_s16[5]);
yprintf(pp,"\n\r\t 12 FET  temperature %5d 0.1K",blk_0x0075_s16[6]);
yprintf(pp,"\n\r\t 14 Max Cell temp    %5d 0.1K",blk_0x0075_s16[7]);
yprintf(pp,"\n\r\t 16 Min Cell temp    %5d 0.1K",blk_0x0075_s16[8]);
yprintf(pp,"\n\r\t 18 Ave Cell temp    %5d 0.1K",blk_0x0075_s16[9]);
yprintf(pp,"\n\r\t 20 CC3 current      %5d userA",blk_0x0075_s16[10]);
yprintf(pp,"\n\r\t 22 CC1 current      %5d userA",blk_0x0075_s16[11]);
yprintf(pp,"\n\r\t 24 CC2 counts   %9d raw ct",(uint32_t)blk_0x0075_s16[12]);
yprintf(pp,"\n\r\t 28 CC3 counts   %9d raw ct",(uint32_t)blk_0x0075_s16[14]);
	return;
}
/* *************************************************************************
 * void bqview_blk_0x0071_u32 (struct SERIALSENDTASKBCB** pp);
 *	@brief	: display parameters: Cell current and voltage counts
 * *************************************************************************/
void bqview_blk_0x0071_u32 (struct SERIALSENDTASKBCB** pp)
{
	int i;

	yprintf(pp,"\n\rDSTATUS1-DSTATUS4 (0x0071-0x0074) current and voltage ADC COUNTS for each cell\n\r        ");
	for (i = 1; i < 17; i++) yprintf(pp,"%8d", i); // Column header

	yprintf(pp,"\n\rvoltage:");
	for (i = 0; i < 16; i++) yprintf(pp,"%8d",cellcts.vi[i].v);

	yprintf(pp,"\n\rcurrent:");
	for (i = 0; i < 16; i++) yprintf(pp,"%8d",cellcts.vi[i].i);

	return;
}
/* *************************************************************************
 * void bqview_blk_0x9335_14 (struct SERIALSENDTASKBCB** pp);
 *	@brief	: display parameters: Balancing items
 * *************************************************************************/
uint8_t blk_0x9335_14[14];
void bqview_blk_0x9335_14 (struct SERIALSENDTASKBCB** pp)
{
yprintf(pp,"\n\rBalancing Configuration - Cell Balance Stop Delta (Relax) (0x9335 - 0x9342) \n\r ");
yprintf(pp,"\n\r\tBalancingConfiguration     0x9335   %02X",           blk_0x9335_14[0]); //Settings:Cell Balancing Config:Balancing Configuration			
yprintf(pp,"\n\r\tMinCellTemp                0x9336 %4d degC",( int8_t)blk_0x9335_14[1]); //Settings:Cell Balancing Config:Min Cell Temp			
yprintf(pp,"\n\r\tMaxCellTemp                0x9337 %4d degC",( int8_t)blk_0x9335_14[2]); //Settings:Cell Balancing Config:Max Cell Temp			
yprintf(pp,"\n\r\tMaxInternalTemp            0x9338 %4d degC",( int8_t)blk_0x9335_14[3]); //Settings:Cell Balancing Config:Max Internal Temp			
yprintf(pp,"\n\r\tCellBalanceInterval        0x9339 %4u sec", (uint8_t)blk_0x9335_14[4]); //Settings:Cell Balancing Config:Cell Balance Interval			
yprintf(pp,"\n\r\tCellBalanceMaxCells        0x933A %4u number",(uint8_t)blk_0x9335_14[5]); //Settings:Cell Balancing Config:Cell Balance Max Cells			
yprintf(pp,"\n\r\tCellBalanceMinCellVCharge  0x933B%5d mv",blk_0x9335_14[6] | blk_0x9335_14[7] << 8); //Settings:Cell Balancing Config:Cell Balance Min Cell V (Charge)			
yprintf(pp,"\n\r\tCellBalanceMinDeltaCharge  0x933D %4u mv",(uint8_t)blk_0x9335_14[8]); //Settings:Cell Balancing Config:Cell Balance Min Delta (Charge)			
yprintf(pp,"\n\r\tCellBalanceStopDeltaCharge 0x933E %4u mv",(uint8_t)blk_0x9335_14[9]); //Settings:Cell Balancing Config:Cell Balance Stop Delta (Charge)			
yprintf(pp,"\n\r\tCellBalanceMinCellVRelax   0x933F%5d mv", blk_0x9335_14[10] | blk_0x9335_14[11] << 8);//Settings:Cell Balancing Config:Cell Balance Min Cell V (Relax)			
yprintf(pp,"\n\r\tCellBalanceMinDeltaRelax   0x9341 %4u mv", blk_0x9335_14[12]); //Settings:Cell Balancing Config:Cell Balance Min Delta (Relax)			
yprintf(pp,"\n\r\tCellBalanceStopDeltaRelax  0x9342 %4u mv", blk_0x9335_14[13]); //Settings:Cell Balancing Config:Cell Balance Stop Delta (Relax)		
	return;
}
/* *************************************************************************
 * void bqview_blk_0x14_u16 (struct SERIALSENDTASKBCB** pp);
 * @brief	: display parameters: Cell voltages
 * *************************************************************************/
extern int16_t cellv[2][BQVSIZE];
extern uint8_t cvidx;
void bqview_blk_0x14_u16 (struct SERIALSENDTASKBCB** pp)
{
	int i;
	int16_t* pv = &cellv[cvidx][0];
    yprintf(pp,"\n\r        ");
    for (i = 0; i < 16; i++) yprintf(pp,"%8d",*pv++);

    return;
}
/* *************************************************************************
 * void bqview_balance1 (struct SERIALSENDTASKBCB** pp);
 * @brief    : display parameters: Cell deviation around average
 * *************************************************************************/
void bqview_balance1 (struct SERIALSENDTASKBCB** pp)
{
	int i;
	int16_t* p = &celldev[0];

	bqcellbal_data ();
	yprintf(pp,"\n\rdev +/- ");
	for (i = 0; i < 16; i++) yprintf(pp,"%8d",*p++);
	return;
}
/* *************************************************************************
 * void bqview_balance_misc (struct SERIALSENDTASKBCB** pp);
 * @brief    : display parameters: Max, min, etc.
 * *************************************************************************/
void bqview_balance_misc (struct SERIALSENDTASKBCB** pp)
{
	yprintf(pp,"\n\rAve:%5d Max%5d cell%3d: Min%5d cell%3d:",cellave,cellmax,cellmaxidx+1,cellmin,cellminidx+1);
	yprintf(pp,"Abs%5d cell%3d:",cellabs,cellabsidx+1);

	//Start balancing cells that are above the written voltage threshold.
yprintf(pp,"\n\rCB_SET_LVL  0x0084: %5d mv",(int16_t)blk_0x0083_u16[1] );

// Reports the number of seconds that balancing has been continuously active.
yprintf(pp,"\n\rCBSTATUS1   0x0085: %5u sec",(uint16_t)blk_0x0083_u16[2] );
	return;
}
/* *************************************************************************
 * void bqview_our_params (struct SERIALSENDTASKBCB** pp);
 * @brief    : display parameters
 * *************************************************************************/
void bqview_our_params (struct SERIALSENDTASKBCB** pp)
{
	struct BQFUNCTION* p = &bqfunction;

	yprintf(pp,"\n\rcellbal: %04X fet_status: %02X battery_status: %02X cellv_high:%5d cellv_low:%5d",
		p->cellbal, p->fet_status, p->battery_status,p->cellv_high,p->cellv_low);
	return;
}
/* *************************************************************************
 * void bqview_our_params_sortV (struct SERIALSENDTASKBCB** pp);
 * @brief    : display parameters
 * *************************************************************************/
void bqview_our_params_sortV (struct SERIALSENDTASKBCB** pp)
{
	extern struct BQCELLV dbsort[NCELLMAX];
	int i;
	for (i = 0; i < 16; i++)
		yprintf(pp,"\n\r%2d %5d %2d",i,dbsort[i].v,dbsort[i].idx);
	return;
}
