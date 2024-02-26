Overview
========
This is a small demo of the high-performance ThreadX kernel. It includes
examples of eight threads of different priorities, using a message queue,
semaphore, mutex, event flags group, byte pool, and block pool. Please
refer to Chapter 6 of the ThreadX User Guide for a complete description
of this demonstration.


SDK version
===========
- Version: 2.14.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Mini/micro USB cable
- MCX-N9XX-EVK board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J5) on the board
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
After writing the program to the flash of the target board,
press the reset button on your board to start the demo.

Example output:
THREADX example ...
start thread 0 ...
start thread 5 ...
start thread 3 ...
start thread 4 ...
start thread 6 ...
start thread 7 ...
start thread 1 ...
start thread 2 ...
