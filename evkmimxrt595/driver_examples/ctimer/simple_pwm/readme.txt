Overview
========
The CTimer Example project is to demonstrate usage of the KSDK ctimer driver.
In this example, CTimer is used to generate a PWM signal.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT595 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J40) on the board
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
The log below shows example output of the CTimer simple PWM demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CTimer example to generate a PWM signal
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Use Oscilloscope to measure and observe the J30-2 output signal to see the ctimer pwm signal.

