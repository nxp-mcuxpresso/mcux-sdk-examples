Overview
========
The SCTimer project is a simple demonstration program of the SDK SCTimer's driver capabiltiy to generate PWM signals.
The SCTimer project enables a 32-bit counter to output two center-aligned PWM signals with parameters set to 24kHz frequency.
The duty cycles of the two PWM signals are 50% and 20%, respectively.

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
- FRDM-RW612 board
- Personal Computer

Board settings
==============
No special settings are required.

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
The log below shows example output of the SCTimer driver simple PWM demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SCTimer example to output 2 center-aligned PWM signals

Probe the signal using an oscilloscope
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Use Oscilloscope to measure and observe the J5-4, J1-1 output signal.
