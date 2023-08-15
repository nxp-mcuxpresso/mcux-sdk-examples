Overview
========
This project is used for MIPI DSI compliant test.

For compliant test, the MIPI DSI works in video mode, there should be 3 types
of data pattern on data lane:

1. ... 11110000 11110000 ...
2. ... 11111000 11111000 ...
3. ... 00000111 00000111 ...

To meet this requirement, the MIPI DSI output pixel format is set to 24-bit format,
and the framebuffer format is XRGB8888. The whole framebuffer is divided into
three parts, one part for each data pattern.

When this project runs, if the panel is connected, the panel shows three regions
with different gray scale. The MIPI DSI signals could be connected to
compliant test instrument.

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVK board
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
    #define DEMO_PANEL DEMO_PANEL_RK055MHD091
    to
    #define DEMO_PANEL DEMO_PANEL_RK055IQH091
    or
    #define DEMO_PANEL DEMO_PANEL_RK055AHD091
    in display_support.h.
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs, the screen shows what described in overview.
