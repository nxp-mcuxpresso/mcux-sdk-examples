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
- USB-C cable
- FRDM-RW612 v2 board
- Personal Computer

Board settings
==============


Prepare the Demo
================
1.  Connect a USB-C cable between the PC host and the MCU-Link USB port (J10) on the board
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
The log below shows example output of the CTimer simple PWM demo using interrupts in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CTimer example to generate a PWM signal
This example uses interrupts to update the PWM duty cycle
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Use Oscilloscope to measure and observe the J1-14 output signal to see the ctimer pwm signal.
