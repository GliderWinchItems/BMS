README-v3.txt

05/27/2023

1. Added PA1 - PA8 connection

This allows the configuration for COMP2-OUT to control
the FET driver. This makes it possible to run the 
charger in CCM mode without TIM1. The hysteresis is
set in the COMP2 register (8, 15, 27 mv options). 

The switching frequency adjusts to the in & out voltages 
and inductor for a given hysteresis selection.

