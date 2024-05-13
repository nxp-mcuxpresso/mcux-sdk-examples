Overview
========
The PXP queue project shows how to use the PXP command queue. This example
use the command queue to rotate the process surface. If this example
runs correctly, you will see a square with three color(red, green and blue).
The rotate degree is changing.

SDK version
===========
- Version: 2.15.100

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- MIMXRT1040-EVK board
- RK043FN02H-CT or RK043FN66HS-CT6 LCD board
- Personal Computer

Board settings
==============
1. Connect the RK043FN02H-CT or RK043FN66HS-CT6 to board.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board. 
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Build the project, the project uses RK043FN66HS-CT6 by default, to use other panels,
    change
    #define DEMO_PANEL DEMO_PANEL_RK043FN66HS
    to
    #define DEMO_PANEL DEMO_PANEL_RK043FN02H
    in emwin_support.h
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs successfully, the log would be seen on the OpenSDA terminal like:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PXP command queue example start...
