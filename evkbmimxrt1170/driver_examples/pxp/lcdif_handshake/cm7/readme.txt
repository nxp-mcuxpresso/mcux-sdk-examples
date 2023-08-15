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

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVKB board
- Personal Computer
- RK055MHD091 panel or RK055AHD091 panel or RK055IQH091 panel

Board settings
==============
Connect the panel to J48
Connect 5V power to J43, set J38 to 1-2

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Build the project, the project uses RK055MHD091 by default, to use other panels,
    change
    #define USE_MIPI_PANEL MIPI_PANEL_RK055MHD091
    to
    #define USE_MIPI_PANEL MIPI_PANEL_RK055IQH091
    or
    #define USE_MIPI_PANEL MIPI_PANEL_RK055AHD091
    in elcdif_support.h.
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs, the screen shows what described in overview.
