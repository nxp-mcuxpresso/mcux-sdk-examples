Overview
========
The MECC project is a simple demonstration program of the SDK MECC driver. It uses single error injection in the on-chip ram and then access the OCRAM to demo single error interrupt.

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

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
The log below shows the output of the hello world demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MECC single error example.

Original ocram data correct.

Single error injection success.

Original ocram data correct.

Single error information success.

Single error address: 0x20240020.

Single error ecc code: 0xc9.

Single error low 32 bits data: 0x44332215.

Single error high 32 bits data: 0x11223344.

Single error bit position of low 32 bits: 0x2.

Single error bit position of high 32 bits: 0x0.

MECC single error example finished successfully.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note: this example need to burn MECC_ENABLE(840[2]) in fuse to enable MECC module, which is disabled by default.
