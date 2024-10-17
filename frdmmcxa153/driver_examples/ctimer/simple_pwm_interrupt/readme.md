Overview
========
The Simple PWM Interrupt project is to demonstrate usage of the SDK CTimer driver as a PWM with interrupt callback functions
In this example an IO pin connected to the LED is used as a PWM output line to generate a PWM signal.
With an interrupt callback the PWM duty cycle is changed frequently in such a way that the LED brightness can be varied.

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
- FRDM-MCXA153 board
- Personal Computer

Board settings
==============
P1_4(J1-2) connect to CTimer simple PWM.

Prepare the Demo
================
1.  Connect a Type-C USB cable between the host PC and the MCU-Link port(J15) on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows example output of the CTimer simple PWM demo using interrupts in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CTimer example to generate a PWM signal
This example uses interrupts to update the PWM duty cycle
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
