README_pcb.txt	

Notes on pcb assembly: bmsmax14921 V1 02/11/2022

1. Options

Opamp:
  R28 1K
  R29 1K
  R30 open
  R31 10M
  TLV521DCKR (MCP6006 not on-hand)

Charger:
  R23 open
  R9  zero
  R7 1.8K
  R110 zero
  R119 110K
  R128 5.76K 

Inductor 47 uh
  RLG0914-470KL-ND (Digikey) 1.56a 140 mOhm

2. Changes needed to eagle .sch, .brd

- R115 3.3K (with R119 68K), and not 33K

V    R1    R2     Vr
85  10    270   3.036
65  3.3   68    3.008
65  3.4   68    3.095
65  3.3   60.4  3.367
65  10    200   3.095
65  10.7  220   3.015
65  5.6   110   3.149
62  5.6   100   3.288
65  5.76  110   3.234

- Selected 5.76K | 110K for install

- FET gate pull-downs (standardize)

  R4. R12. R14 -> 1M
  R1, R11, R19, R124 100K

- R125 pads too close to FET

- MIC5213 footprint err: 123 & 56 pads too far apart

- MCP6006 might have footprint err, 
  TVL521DCKR worked, but was slightlys smaller (SC70?)

- D5 pads for SOD (smd rather than barrel type)

- Solder mask offset on spark-gap

- Missing part data on some BOM listings

- All FETs need +/- Vgs (re: FET104 where selected part
  had a +/- 16v which does't leave headroom)


- ITEM: Vl connected to 3.3v would eliminate the need
  for the level shifting.
  
  05/13/22

  MCP5213 LDO

  - SC-70 pad spacing problem
  - Droppng resistor (100K) must be about
     1K for it to regulate
     330 ohms installed R26
     zero on R23 (this could be 10K?)
  - Current drain, no load, about 86 ua.

  Substitute STLQ050, requires pin assignment
  change (as well as package pads assignments, etc.)


