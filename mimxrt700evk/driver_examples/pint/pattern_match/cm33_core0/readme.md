Overview
========
This example shows how to use SDK drivers to use the Pin interrupt & pattern match peripheral.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- MIMXRT700-EVK board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-LINK USB port on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
1. The following lines are printed to the serial terminal when the demo program is executed.

PINT Pattern Match example

PINT Pattern match events are configured

Press corresponding switches to generate events

2. This example configures "Pin Interrupt 0" to be invoked when SW5 switch is pressed by the user. 
   Bit slice 0 is configured as an endpoint in sticky falling edge mode. The interrupt callback prints 
   "PINT Pin Interrupt 0 event detected. PatternMatch status =        1". 
   
   "Pin Interrupt 2" is configured to be invoked when rising edge on SW5, SW7 is detected. The 
   interrupt callback prints "PINT Pin Interrupt 2 event detected. PatternMatch status =     100". Bit slices
   1 configured to detect sticky rising edge. Bit slice 2 is configured as an endpoint.

