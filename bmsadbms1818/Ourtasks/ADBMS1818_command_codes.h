/******************************************************************************
* File Name          : ADBMS1818_command_codes.h
* Date First Issued  : 006/12/2022
* Board              : 
* Description        : Command codes from datashseet Table 52
*******************************************************************************/

#ifndef __ADBMS_COMMAND_CODES
#define __ADBMS_COMMAND_CODES

#define WRCFGA   0x001  // Write Configuration Register Group A
#define WRCFGB   0x024  // Write Configuration Register Group B
#define RDCFGA   0x002  // Read Configuration Register Group A
#define RDCFGB   0x026  // Read Configuration Register Group B
#define RDCVA    0x004  // Read Cell Voltage Register Group A
#define RDCVB    0x006  // Read Cell Voltage Register Group B
#define RDCVC    0x008  // Read Cell Voltage Register Group C
#define RDCVD    0x00A  // Read Cell Voltage Register Group D
#define RDCVE    0x009  // Read Cell Voltage Register Group E
#define RDCVF    0x00B  // Read Cell Voltage Register Group F
#define RDAUXA   0x00C  // Read Auxiliary Register Group A
#define RDAUXB   0x00E  // Read Auxiliary Register Group B
#define RDAUXC   0x00D  // Read Auxiliary Register Group C
#define RDAUXD   0x00F  // Read Auxiliary Register Group D
#define RDSTATA  0x010  // Read Status Register Group A
#define RDSTATB  0x012  // Read Status Register Group B
#define WRSCTRL  0X014  // Write S Control Register Group
#define WRPWM    0x020  // Write PWM Register Group
#define WRPSB    0x01C  // Write PWM/S Control Register Group
#define RDSCTRL  0x016  // Read S Control Register Group
#define RDPWM    0x022  // Read PWM Register Group
#define RDPSB    0x01E  // Read PWM/S Control Register Group B
#define STSCTRL  0x019  // Start S Control Pulsing and Poll Status
#define CLRSCTRL 0x018  // Read PWM/S Control Register Group B

#define ADCV     0x360  // Start Cell Voltage ADC Conversion and Poll Status
#define ADOW     0x368  // Start Open Wire ADC Conversion and Poll Status
#define CVST     0x347  // Start Self Test Cell Voltage Conversion and Poll Status
#define ADOL     0x301  // Start Overlap Measurements of Cell 7 and Cell 14 Voltages
#define ADAX     0x560  // Start GPIOs ADC Conversion and Poll Status
#define ADAXD    0x500  // Start GPIOs ADC Conversion with Digital Redundancy and Poll Status
#define AXOW     0x550  // Start GPIOs Open Wire ADC Conversion and Poll Status
#define AXST     0x547  // Start Self Test GPIOs Conversion and Poll Status
#define ADSTAT   0x568  // Start Status Group ADC Conversion and Poll Status
#define ADSTATD  0x508  // Start Status Group ADC Conversion with Digital Redundancy and Poll Status
#define STATST   0x54F  // Start Self Test Status Group Conversion and Poll Status
#define ADCVAX   0x56F  // Start Combined Cell Voltage and GPIO1, GPIO2 Conversion and Poll Status
#define ADCVSC   0x567  // Start Combined Cell Voltage and SC Conversion and Poll Status 

#define CLRCELL  0x711  // Clear Cell Voltage Register Groups
#define CLRAUX   0x712  // Clear Auxiliary Register Groups
#define CLRSTAT  0x713  // Clear Status Register Groups
#define PLADC    0x714  // Poll ADC Conversion Status
#define DIAGN    0x715  // Diagnose MUX and Poll Status

#define WRCOMM   0x721  // Write COMM Register Group
#define RDCOMM   0x722  // Read COMM Register Group
#define STCOMM   0x723  // Start I2C/SPI Communication
#define MUTE     0x028  // Mute Discharge
#define UNMUTE   0x028  // Unmute Discharge

#endif
