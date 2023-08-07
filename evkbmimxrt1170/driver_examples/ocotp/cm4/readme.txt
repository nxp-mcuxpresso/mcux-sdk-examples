Overview
========

The OCOTP example project is a demonstration program that uses the KSDK software to access eFuse map.
Consider of the feature of One-Time programable, this example will just print the version of OCOTP controller.

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
The log below shows the output of the hello ocotp example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OCOTP Peripheral Driver Example

OCOTP controller version: 0x0A000000

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
