Overview
========
This is a small demo of the high-performance ThreadX kernel. It includes
examples of eight threads of different priorities, using a message queue,
semaphore, mutex, event flags group, byte pool, and block pool. Please
refer to Chapter 6 of the ThreadX User Guide for a complete description
of this demonstration.


Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.7.0

Hardware requirements
=====================
- A Micro USB cable
- MIMXRT1170-EVK board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Compile the demo.
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

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
