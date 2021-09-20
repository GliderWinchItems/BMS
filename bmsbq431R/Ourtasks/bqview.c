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

#include "bqview.h"
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
yprintf(pp,"\n\rIntTemperature     0x68 0x%5d",(int16_t)blk_0x62_u16[ 3]-2732); // Int Temperature: most recent measured internal die temperature
yprintf(pp,"\n\rCFETOFFTemperature 0x6A 0x%5d",(int16_t)blk_0x62_u16[ 4]     ); // Config: thermistor-most recent temp (0.1K); ADCIN-reading in mv
yprintf(pp,"\n\rDFETOFFTemperature 0x6C 0x%5d",(int16_t)blk_0x62_u16[ 5]     ); // Config: thermistor-most recent temp (0.1K); ADCIN-reading in mv 
yprintf(pp,"\n\rALERTTemperature   0x6E 0x%5d",(int16_t)blk_0x62_u16[ 6]     ); // Config: thermistor-most recent temp (0.1K); ADCIN-reading in mv
yprintf(pp,"\n\rTS1Temperature     0x70 0x%5d",(int16_t)blk_0x62_u16[ 7]     ); // Config: thermistor-most recent temp (0.1K); ADCIN-reading in mv
yprintf(pp,"\n\rTS2Temperature     0x72 0x%5d",(int16_t)blk_0x62_u16[ 8]     ); // Config: thermistor-most recent temp (0.1K); ADCIN-reading in mv
yprintf(pp,"\n\rTS3Temperature     0x74 0x%5d",(int16_t)blk_0x62_u16[ 9]     ); // Config: thermistor-most recent temp (0.1K); ADCIN-reading in mv
yprintf(pp,"\n\rHDQTemperature     0x76 0x%5d",(int16_t)blk_0x62_u16[10]     ); // Config: thermistor-most recent temp (0.1K); ADCIN-reading in mv
yprintf(pp,"\n\rDCHGTemperature    0x78 0x%5d",(int16_t)blk_0x62_u16[11]     ); // Config: thermistor-most recent temp (0.1K); ADCIN-reading in mv
yprintf(pp,"\n\rDDSGTemperature    0x7A 0x%5d",(int16_t)blk_0x62_u16[12]     ); // Config: thermistor-most recent temp (0.1K); ADCIN-reading in mv
yprintf(pp,"\n\rFETStatus          0x7F 0x%02X",blk_0x62_u16[13]); // FET Status: flags showing status of FETs and ALERT pin
	return;
}