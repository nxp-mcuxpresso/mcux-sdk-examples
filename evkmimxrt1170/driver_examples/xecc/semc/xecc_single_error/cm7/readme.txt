Overview
========
The XECC project is a simple demonstration program of the SDK XECC driver. It uses single error injection in the external memory
and then access the external memory to demo single error interrupt.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.4.0

Hardware requirements
=====================
- Mini/micro USB cable
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
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs successfully, the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
XECC Single Error Example Start!
Correcdted read data: 0xddccbbaa 
Uncorrecdted read data: 0xddccbbae 
Single error address: 0x80000000 
Single error read data: 0xddccbbae 
Single error ECC code: 0x22cc44aa 
Single error bit postion: 0x4 
Single error bit field: 0x1 
XECC single error example successfully!

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note: this example need to burn XECC_ENABLE(840[3]) in fuse to enable XECC module, which is disabled by default.
