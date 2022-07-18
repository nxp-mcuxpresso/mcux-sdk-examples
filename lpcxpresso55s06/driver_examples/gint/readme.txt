Overview
========
This example shows how to use SDK drivers to use the Group GPIO input interrupt peripheral.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso55S06 board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J1) on the board
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
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

2. "Group GPIO interrupt 1"" is configured to be invoked when both the switches SW1 and SW4 are pressed at the same time. The interrupt callback prints "GINT1 event detected" message on the serial terminal.
   This example configures "Group GPIO interrupt 0"" to be invoked when SW3 switch is pressed by the user. The interrupt callback prints "GINT0 event detected" message on the serial terminal.
