Overview
========
The sema4 basic example shows how to use SEMA4 driver to do basic gate operaions,
including lock, unlock, and reset.

Toolchain supported
===================
- MCUXpresso  11.6.0
- GCC ARM Embedded  10.3.1

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
When the example finished successfully, you can see the log
"SEMA4 basic example finished successfully" in the terminal window. If the example
runs failed, you can see the log like "Example error, line: xxx".
