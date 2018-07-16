README.txt
06/24/2018

powerbox

STM32F103C8T6 Arduino "Blue Pill " board

One-off board with MOV, TVS, fuse, 0.22 ohm 
current limiting resistor.

CAN bus interface board with LM2596

Monitor voltages_
  5v dc-dc switcher output
  Raw input
  After Diode, before series resistor
  After resistor, Big Capacitor, CAN bus
  Hall-effect current sensor

CAN bus messages:
  Hearbeat: input voltage
  Alarm: input voltage below threshold (10.5v)
   every x (50 ms).
  Hall effect current: 64/sec triggered by time sync
   message.


Hall effect notes: 07/15/2018

Flying lead 6P6C jack (FLJ) and wiring to Tamura L01Z is intended to
match the jack wiring for the pcb 'fethe' (in directory
 ../embed/svn_sensor/hw/trunk/eagle/fethe)

Color coding of flying lead jack, purchased 6P6C cables, and
Blue Pill wiring--

Pin desc  FLJ  Cable BPill
1 GND FET wht  gry   --
2 FET DRV blk  yel   --
3 HE GND  red  grn   blk
4 HE SIG  grn  red   grn
5 +5v     yel  blk   red
6 +12V    blu  wht   --


