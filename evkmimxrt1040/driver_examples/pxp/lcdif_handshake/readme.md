Overview
========
The PXP LCDIF hand shake project shows how to enable the hand shake between
PXP and LCDIF. If this example runs correctly, you will see two rectangles moving in the
screen. One rectangle is process surface output, and the other is alpha surface
output, the overlay region color is OR'ed value of process surface and alpha surface.

NOTE:
This example must run continously to get the right result, breaking during debug
will results to handshake fail. Because the PXP must be started before every LCD
frame, if system is halt, the PXP does not start correctly.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

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
PXP LCDIF hand shake example start...
