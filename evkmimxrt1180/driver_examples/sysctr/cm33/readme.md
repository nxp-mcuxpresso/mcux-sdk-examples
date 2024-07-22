Overview
========

The sysctr example shows the usage of System Counter driver in application. The System Counter provides
a shared time base to multiple processors. It is intended for applications where the counter is always
powered on, and supports multiple unrelated clocks.

In this example, System Counter is clocked by 24MHz base clock. System Counter count value will be printed
at 3 second and 5 second.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1180-EVK board
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
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
When the demo runs successfully, will get the similar messages on the terminal.

~~~~~~~~~~~~~~~~~~~~~~
Starting System Counter ...
 System Counter compare interrupt is occurred !
 System Counter upper 24 bits is 0
 System Counter lower 32 bits is 44aa20a
 System Counter compare interrupt is occurred !
 System Counter upper 24 bits is 0
 System Counter lower 32 bits is 7270e0a
~~~~~~~~~~~~~~~~~~~~~~
