Overview
========
This demo describes how to use SDK drivers to implement the PWM feature by FLEXIO IP module.
It outputs the PWM singal with fixed frequency defined by "DEMO_FLEXIO_FREQUENCY" in source code
and dynamic duty from 99 to 1 to one of the FLEXIO pins.

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
- FRDM-MCXN236 board
- Personal Computer

Board settings
==============
PWM output: J8-11(P4_19, FLEXIO_D27)

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the EVK board J10.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.
5.  Use oscilloscope to measure the output 100KHz PWM signal pin at J8-11 pin.

Running the demo
================
The following lines are printed to the serial terminal when the demo program is executed.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FLEXIO_PWM demo start.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
