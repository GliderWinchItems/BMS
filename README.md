# BMS
Battery Management System

This repo has directories for three BMS designs.

The original plan used a LTC6813-1 (18 cells), but when it came time to order the pcb, the part was no longer available. The TI BQ76952 (16 cells) was then found to be available in a few months so that design was completed.
The testing of the first prototype with the BQA76952 turned up some unexplained failures, plus additional BQ76952 parts would not be available for approximately one year. 

The ADBMS1818 chip became available and samples were to be shipped in a few weeks, and additional parts available from Mouser in several months. This gave rise to the second design. The delivery schedule changed  and it would be about one year for samples and additional parts. This gave rise to a design based on the MAX14952.
The designs differ mainly in the BMS chip. The same processor, STM32L431RxT6, is used and the features are  nearly identical in the three designs, e.g. the trickle charger, CAN, and fan interfaces, etc. Much of the documentation for the BQ76952 board applies to all three designs.
