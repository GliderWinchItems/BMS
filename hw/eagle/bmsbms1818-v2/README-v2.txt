README-v2.txt
08/16/2022

bms1818-v2 starts with the bms1818 eagle files and modifies
them.

1. Missing +5/V1 missing to thermistor pullups fixed.

2. Renumbering of parts top & bottom (with +100).

After automatic renumbering the associated cell resistors,
capacitors, and FETs renumbered so that they reflect the
cell number.

3. Via added for easy connection to tjhe ADBMS1818 SDO line.


4. TODO

Change GPIO assignments so that the Op-Amp current sense output
goes to GPIO2. That way the ADCVAX command will measure the
cells and GPIO1 and GPOI2 which reduces the measurement
offset of the current versus voltage for the op-amp sensing.

