Overview
========
This example shows how to use SDK drivers to use the Pin interrupt & pattern match peripheral.

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- LPC845 Breakout board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the demo
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port on the board.
2.  Open a serial terminal with the following settings:
    - 9600 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The following lines are printed to the serial terminal when the demo program is executed.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PINT Pattern Match example

PINT Pattern match events are configured

Press corresponding switches to generate events
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This example configures "Pin Interrupt 0" to be invoked when K1 is pressed by the user.
Bit slice 0 is configured as an endpoint in sticky falling edge mode. The interrupt callback prints 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
"PINT Pin Interrupt 0 event detected. PatternMatch status =        1". 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
"Pin Interrupt 2" is configured to be invoked when K3 is pressed. The interrupt callback prints
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
"PINT Pin Interrupt 2 event detected. PatternMatch status =      100".
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
