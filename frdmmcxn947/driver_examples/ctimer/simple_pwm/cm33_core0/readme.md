Overview
========
The CTimer Example project is to demonstrate usage of the KSDK ctimer driver.
In this example, CTimer is used to generate a PWM signal.

SDK version
===========
- Version: 2.16.000

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
P0_10(J2-4) connect to CTimer simple PWM.

Prepare the Demo
================
1.  Connect a type-c USB cable between the PC host and the MCU-Link USB port (J17) on the board
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows example output of the CTimer simple PWM demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CTimer example to generate a PWM signal
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
