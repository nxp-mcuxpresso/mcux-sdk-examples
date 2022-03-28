Overview
========

This project shows how to use the OPAMP driver. In this example, both positive input and
negative input should be connected to GND, the positive reference voltage is set as DAC
output, the OPAMP output is 1X of DAC output. When DAC output changes, the OPAMP changes
accordingly.
The calculating formula is:
    OUT =   ((NGAIN + 1) / (1 + 1/PGAIN)) * Vinp - (NGAIN * Vinn) + ((1 + NGAIN) / (1 + PGAIN)) * Vpref

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.4.1

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

