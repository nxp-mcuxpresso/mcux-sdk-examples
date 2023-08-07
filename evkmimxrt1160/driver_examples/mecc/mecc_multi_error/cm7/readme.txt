Overview
========
The MECC project is a simple demonstration program of the SDK MECC driver. It uses multiple error injection in the on-chip ram and then access the OCRAM to demo multiple error interrupt.

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1160-EVK board
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

Multiple error address: 0x20240020.

Multiple error ecc code: 0xc9.

Multiple error low 32 bits data: 0x44332215.

Multiple error high 32 bits data: 0x11223340.

MECC multiple error example finished successfully.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note: this example need to burn MECC_ENABLE(840[2]) in fuse to enable MECC module, which is disabled by default.
