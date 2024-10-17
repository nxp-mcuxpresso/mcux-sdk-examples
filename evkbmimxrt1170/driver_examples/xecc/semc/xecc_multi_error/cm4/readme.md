Overview
========
The XECC project is a simple demonstration program of the SDK XECC driver. It uses double error detection in the external memory
and then access the external memory to demo multiple error interrupt.

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
- MIMXRT1170-EVKB board
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
When the demo runs successfully, the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
XECC Multiple Errors Example Start!
First level BusFault interrupt finished.
Uncorrecdted read data: 0xddccbba9 
Multiple error address: 0x80000000 
Multiple error read data: 0xddccbba9 
Multiple error ECC code: 0x22cc44aa 
Multiple error bit field: 0x1 
XECC multiple errors example successfully!

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note: this example need to burn XECC_ENABLE(840[3]) in fuse to enable XECC module, which is disabled by default.
