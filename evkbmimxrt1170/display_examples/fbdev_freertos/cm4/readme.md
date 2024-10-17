Overview
========
This example shows how to use the fbdev component. The fbdev masks
the difference of LCD controllers and provides a unified APIs for upper
layers. At the same time, the frame buffers are managed by the FBDEV.
When the example runs, you can see a rectangle moving in the screen, and
its color changes when reached the border.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVKB board
- Personal Computer
- RK055MHD091 panel or RK055AHD091 panel or RK055IQH091 panel or RaspberryPi panel

Board settings
==============
Connect the panel to J48. For RaspberryPi panel, connect the panel to J84,
then connect the panel's 5V pin to J85-1, GND pin to J85-2.
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
    or
    #define USE_MIPI_PANEL DEMO_PANEL_RASPI_7INCH
    in display_support.h.
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs, the screen shows what described in overview.
