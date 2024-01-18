Overview
========
This is a small demo of the high-performance ThreadX kernel. It includes
examples of eight threads of different priorities, using a message queue,
semaphore, mutex, event flags group, byte pool, and block pool. Please
refer to Chapter 6 of the ThreadX User Guide for a complete description
of this demonstration.


SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- One Micro USB cables
- Target Board
- Personal Computer(PC)

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB Micro cable between the host PC and the Debug Link USB port (J1) on the target board.
2.  Open a serial terminal on PC with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Compile the demo.
4.  Download the program to the target board.
5.  Press the on-board RESET button to start the demo.

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
