Overview
========

The QTMR project is a demonstration program of the SDK QTMR driver's input capture and output pwm feature.
The example sets up a QTMR channel for input capture. Once the input signal is received, the example will print
the capture frequency value while the check is interrupted and waiting for the capture frequency to stabilize.
The example also sets up one QTMR channel to output pwm. The user should probe a 50Khz PWM signal output with
50% dutycycle with a oscilloscope. 

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1180-EVK board
- Personal Computer
- Oscilloscope

Board settings
==============
1. Input PWM signal to J45-15. PWM signal: 0-3.3V, default 1KHz.
2. Use oscilloscope to monitor the signal at J45-13.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board. 
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
When the demo runs successfully, the following message is displayed in the terminal:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

****Input capture example.****

****Provide a signal input to the QTMR pin****

Captured Period time=1000 us

****Output PWM example.****

*********Make sure to connect an oscilloscope.*********

****A 50 duty cycle PWM wave is observed on an oscilloscope.****

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
