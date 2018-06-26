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


