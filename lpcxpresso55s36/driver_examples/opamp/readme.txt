Overview
========

This project shows how to use the OPAMP driver. In this example, the OPAMP
work in internal gain mode, the OPAMP output is set to 2X of DAC output.
When DAC output changes, the OPAMP output changes accordingly.

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/Micro USB cable
- LPCXpresso55S36 board
- Personal Computer

Board settings
==============
DAC output pin: J12-4

Prepare the Demo
================
1.  Connect a USB cable between the PC host and the OpenSDA USB port on the board.
2.  Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.
5.  A multimeter may be used to measure the DAC output voltage.

Running the demo
================
OPAMP EXAMPLE!

Please input a value (0 - 4095) for DAC:

