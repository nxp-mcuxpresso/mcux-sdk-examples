Overview
========
This example shows how to use SDK drivers to use the Group GPIO input interrupt peripheral.

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso54628 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a micro USB cable between the host PC and the LPC-Link USB port (J8) on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.

Running the demo
================
1.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

The following lines are printed to the serial terminal when the demo program is executed.

Group GPIO input interrupt example

GINT0 and GINT1 events are configured

Press corresponding switches to generate events

2. This example configures "Group GPIO interrupt 0"" to be invoked when SW2 switch is pressed by the user.
   The interrupt callback prints "GINT0 event detected" message on the serial terminal. ""Group GPIO 
   interrupt 1"" is configured to be invoked when both the switches SW3 and SW4 are pressed at the same time.
   The interrupt callback prints "GINT1 event detected" message on the serial terminal.
