README_pcb.txt	

Notes on pcb assembly: BQ V1 20210222

08/27/2021

1. PYJB dc-dc converter bottom view instead of top view

2. 4.7u 25v or greater ceramic caps pads need to be larger

- 0805 probably OK. 
- Some 25v are 1206, but can be sweated on to 0603 w 
  hot air workstation.
- C123, C121, and another?

3. R5 0.1 ohm smd input current sense resistor

Add smd pads
Convert current holes to a 2 pin header

4. Add 18M pads across 32 KHz xtal

5. C112 (1u, 100v smd) too close to FET
-move away from FET case

6. D6 Zener leads hole should be larger
- barely force them thru.

7. R5 0.1 current sense: change to 2 pin header
- header with jumper
- remove jumper and put ammeter across
- Or, add smd pads for smd sense resistor
  and use header for millivolt meter access

8. D109 (100v 2a Schottky)

- change pad to 2 pin from 3 pin.
- Angle part 

9. FET2 - Pack Heater FET

- Add a gate driver. Low gate threshold
  FETs are marginal.

10. Q1 - BJT transistor pad is wrong

- BCX56-16TF: Should be SOT-89-3 not SOT-123
- Thru-hole could be considered

11. 12v FAN keyed header

- Is this a top/bottom problem?

12. JP17 too close to IDC connector

- reposition FET100 header

13. Need "GND" mark on (at least) the following

JP14 (SWD)
JP10 (I2C1)
JP18
JP21
JP13 (Ext LED)
JP1 DUMP
JP16 DUMP2

14. Some label changes

Pushbutton: U$1 -> "reset"

15. Repositioning--

- JP6 to close to ground via
- C4 too close to FET

16. Options installed

R110 - Zero [by-pass op-amp]
R7   - 1.8K [schematic calls for 1.5K]
C4   - 10u 25v
R9   - zero
R23  - open
R119 - open
R128 - 68K [schematic calls for 64K]
R115 - 33K

17. Initial 

No 12v regulator installed
'L431 running

Across 10 ohm - 341.3mv
Voltage - 13.23
current = 34.13 ma
power   = 451 mw


