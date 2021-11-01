README-cable-wiring
10/25/2021

1. Adapter RJ-45 <-> 2x5 BMS header

  2X5 keyed BMS CAN header
   1 +12v
   2 GND
   3 CAN-L
   4 CAN-H
   5 GND
   6 Master-reset
   7 +12v
   8 +12v
   9 GND
  10 nc

  RJ45
  1 carry-thru
  2 +12v
  3 +12v
  4 CAN-H
  5 CAN-L
  6 GND
  7 GND
  8 Master-reset

2. 2x5 keyed BMS header <-> DE9

Based on ribbon cable crimp to DE9

  col 1: 2x5 header pin number
  col 2: 2x5 signal name
  col 3: DE9 pin number
  col 4: DE9 standard CAN usage signal name

   1 +12v........... 1 Reserved
   2 GND............ 6 GND
   3 CAN-L.......... 2 CAN-L
   4 CAN-H.......... 7 CAN-H
   5 GND............ 3 CAN GND
   6 Master-reset... 8 Reserved
   7 +12v........... 4 Reserved
   8 +12v........... 9 CAN V+
   9 GND............ 5 CAN shield
  10 nc

3. 2x10 keyed BMS ribbon cable: 18 cell layout

col 1: 2x10 header pin number
col 2: 20 pin ribbon color
col 3: module cell number
col 4: additional description

---------TOP SPLIT-------------
 1 BLK - Current sense (to current sense op-amp)
 2 WHT - GND/Vss
 3 GRY - C2
 4 PUR - C4
 5 BLU - C6
 6 GRN - C8
 7 YEL - C10
 8 ORG - C12
 9 RED - C14
10 BRN - C16
11 BLK - C18
----------BOTTOM SPLIT ---------
12 WHT - C1
13 GRY - C3
14 PUR - C5
15 BLU - C7
16 GRN - C9
17 YEL - C11
18 ORG - C13
19 RED - C15
20 BRN - C17

4. Adapter 18 cell ribbon to 16 cell BQ BMS

col 1: 18 cell: 2x10 header pin number 18 cell layout
col 2: 18 cell: 20 pin ribbon color
col 3: 18 cell: module cell
col 4: BQ 2x10 header pin number (16 cells)
COL 5: BQ cell number

-Ribbon top split-
 1 BLK - Current  1, 3, 4
 2 WHT - GND/Vss  2
 3 GRY - C2       5 C2
 4 PUR - C4       6 C4
 5 BLU - C6       7 C6
 6 GRN - C8       8 C8
 7 YEL - C10       open
 8 ORG - C12      9 C10  
 9 RED - C14     10 C12
10 BRN - C16     11 C14
11 BLK - C18     12 C16
-Ribbon bottom split-
12 WHT - C1      13 C1
13 GRY - C3      14 C3
14 PUR - C5      15 C5
15 BLU - C7      16 C7
16 GRN - C9       open
17 YEL - C11     17 C9
18 ORG - C13     18 C11
19 RED - C15     19 C13
20 BRN - C17     20 C15

