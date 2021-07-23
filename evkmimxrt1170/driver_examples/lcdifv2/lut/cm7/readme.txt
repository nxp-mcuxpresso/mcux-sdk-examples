Overview
========
This example demonstrates how to use the LCDIF v2 color palette (LUT). In this
example the pixel format is 8-bit LUT index. Although the LUT supports 256 items
in this example only items 0-7 are used, because the frame buffer content are
restricted to 0-7.

When the example runs, a rectangle is moving in the screen, and
its color changes when touch border.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.4.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVK board
- Personal Computer
- RK055AHD091 panel or RK055IQH091 panel

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
3.  Build the project, the project uses RK055AHD091 by default, to use panel RK055IQH091,
    change
    #define USE_MIPI_PANEL MIPI_PANEL_RK055AHD091
    to
    #define USE_MIPI_PANEL MIPI_PANEL_RK055IQH091
    in lcdifv2_support.h.
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs, the screen shows what described in overview.
