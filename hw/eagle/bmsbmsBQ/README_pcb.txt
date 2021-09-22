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
  
   No. See test problem below. 18M hung 'init'.

5. C112 (1u, 100v smd) too close to FET
-move away from FET case

6. D6 Zener leads hole should be larger for 5W size
- 1N5375BG: barely force them thru.
- OK for 83v DO-3 size

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

18. LED resistor size change
         P/N           Digikey
RED - 150060RS75000 - 732-4978-1-ND 0603
GRN - 150060GS75000 - 732-4971-1-ND -6-3

Red led less output than Grn
GRN - R15 - 6.8K - 8.2K
RED - R16 - 2.2K - 3.3K


=========== TEST & CHECKOUT ============
09/05/2021

1. LEDs

- resistor change to equalize light output
- RED led replaced. Low output. May have been
  damaged probing and applying 3.3v to it.

2. Clocking

- 8 MHz xtal works

- 32 KHz was hanging 'init'
  Removed 18M feedback resistor and it
  started to work.

  Switch 'MX back to LSI jic xtal doesn't
  start, since 32 KHz timer not currently
  needed.

3. morse.c

- Revise to allow selecting RED, GRN, BOTH

- 'morse_trap' does BOTH

- Speed based on 16MHz clock

4. Tactile Reset PB works

5. FET Dump, Dump2, & Heater: on/off

- fetonoff.c: deals with i/o pin set/reset

- On/Off program control works as far as
  applying gate voltage to the fets.

- DUMP & HEATER FETs driven from Gate IC1:
  5V/3 comes from BQ Reguator so if it is
  missing the FET drive is about 2.5v which
  comes via the processor I/O pins.

  Without the BQ Regulator on this is not
  enough drive. However, if there is battery
  voltage the BQ will be able to run the 5V/3
  regulator output.

- Increase R102 for DUMP FET100 
  If FET doesn't turn full-on the 1 ohm allows
  too much current and the FET quickly overheats.
  Make it 10K; the same as FET101 (DUMP2)

6. BAT-CP1

This needs to be powered from C16 (though maybe
it could be a lower cell).

================= BQ powering

The external NPN BJT used for the REGIN preregulator can be configured with its collector routed either to
the cell battery stack or the middle of the protection FETs.
A diode is recommended in the drain circuit of the external NPN BJT, which avoids reverse current flow from
the BREG pin through the BJT base to collector in the event of a pack short circuit. This diode can be a
Schottky diode if low voltage pack operation is needed, or a conventional diode can be used otherwise.
A series diode is recommended at the BAT pin, together with a capacitor from the pin to VSS. These
components allow the device to continue operating for a short time when a pack short circuit occurs, which
may cause the PACK+ and top-of-stack voltages to drop to approximately 0 V. In this case, the diode
prevents the BAT pin from being pulled low with the stack, and the device will continue to operate, drawing
current from the capacitor. Generally operation is only required for a short time, until the device detects the
short circuit event and disables the DSG FET. A Schottky diode can be used if low voltage pack operation is
needed, or a conventional diode can be used otherwise.
The diode in the BAT connection and the diode in the BJT collector should not be shared, since then the
REG0 circuit might discharge the capacitor on BAT too quickly during a short circuit event.

7. PD2 LD does not appear appropriate!

Maybe it does, as it is used for BQ wakeup.

(Remove and connect to 10K resistor to PACK.)

8. Cut 5V/3 (BQ REG1) and route to 5v/1 (linear reg)

5v needed for heater and dump fets drive. FETs wouldn't
be used without the processor running, (unless the BQ is set to
run autonomously).

9. Pack pin  via 10K to top-of-stack.

See Datasheet 16.2 typical applications

10. Rev-2: Beeper?

Shutdown/wakeup once per minute. Beep if some cell is getting
too low.

11. Alert pin

Needs pullup to 3.3v, unless REG1 is used for BQ internal pullup.

OR--use L431 internal pullup.

12. thermistor headers too close 

The OTS cabled thermstors have a flange on the 
2-pin connectors and need more spacing between headers.

Work-around: use alternate connectors.

13. BAT voltage with no battery cable 

Should there be a diode OR of battery voltage and voltage that goes
to the 3.3v regulator? This would allow communication with the BQ
when the battery cable is not plugged in.

14. Header pads for battery cable connecting to discharger board

Removing these adds board space, lengthwise.








  





