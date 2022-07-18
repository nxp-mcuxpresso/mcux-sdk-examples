Overview
========
The ELCDIF LUT project shows how to use the ELCDIF LUT to convert 8-bit input pixel
to 24-bit output pixel. There are two LUT memories, this project uses one input
frame buffer, and swithes between the two memories, so from the LCD panel, two
different pictures are shown one by one.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1160-EVK board
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
