Overview
========
The MECC project is a simple demonstration program of the SDK MECC driver. It uses multiple error injection in the on-chip ram and then access the OCRAM to demo multiple error interrupt.

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
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows the output of the hello world demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MECC multiple error example.

Original ocram data correct.

Multiple error injection success.

Multiple error injection success.

Multiple error information success.

Multiple error address: 0x20500020.

Multiple error ecc code: 0xc9.

Multiple error low 32 bits data: 0x44332215.

Multiple error high 32 bits data: 0x11223340.

MECC Multiple error example finished successfully.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
