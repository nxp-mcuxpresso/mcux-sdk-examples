Overview
========

The OPAMP trigger mode example demonstrates how to use SCTIMER to trigger OPAMP. 
In this example, INP0 and INP1 are connected to different levels. When OPAMP 
receives a trigger signal, it will switch to different output voltages.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXN947 board
- Personal Computer

Board settings
==============
Positive INP0 J4-1(P4_12).
Positive INP1 J8-18(P4_13).
Negative input channel J4_3(OPAMP0_INN) connects to GND.
OPAMP output pin: J8-20(P4_15)

Prepare the Demo
================
1.  Connect a type-c USB cable between the PC host and the CMSIS DAP USB port (J17) on the board
2.  Open a serial terminal with the following settings (See Appendix A in the Getting started guide for a description of how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One-stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
OPAMP TRIGGER MODE EXAMPLE!

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
1. Connect INP0 and INP1 to different levels.
2. Monitor OPAMP output with an oscilloscope. When OPAMP receives a trigger signal, it will switch to different output voltages.
