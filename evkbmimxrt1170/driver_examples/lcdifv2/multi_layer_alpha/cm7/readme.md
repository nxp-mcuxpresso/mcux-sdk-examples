Overview
========
This example demonstrates how to use the LCDIF v2 API to get the global alpha
based on desired blended alpha, for the multi-layer blend case.

In this example, 7 layers are enabled. Layer 0 is full screen blue color background,
layer 1-7 are alpha layers, they are all green with size (PANEL_WITDH / 2) x (PANEL_HEIGHT / 4).
The posision of each layer is:

+-----------------+-------------------+
|                 |                   |
|                 |                   |
|    Layer 1      |    Layer 2        |
|                 |                   |
|                 |                   |
+-----------------+-------------------+
|                 |                   |
|                 |                   |
|    Layer 3      |    Layer 4        |
|                 |                   |
|                 |                   |
+-----------------+-------------------+
|                 |                   |
|                 |                   |
|    Layer 5      |    Layer 6        |
|                 |                   |
|                 |                   |
+-----------------+-------------------+
|                 |                   |
|                 |                   |
|    Layer 7      |                   |
|                 |                   |
|                 |                   |
+-----------------+-------------------+

The alpha layers share the same frame buffer, and the desired blended alpha are all 31.
LCDIFv2 API is called to calculate the global alpha for each layer to get the desired
blended alpha.

Because the blended alpha for each alpha layer is the same, so all alpha layers result are the same,
the panel looks like:

+-------------------------------------+
|                                     |
|                                     |
|                                     |
|                                     |
|                                     |
|       Layer 1 ~ 7, all cyan         |
|                                     |
|                                     |
|                                     |
|                                     |
|                                     |
|                                     |
|                                     |
|                                     |
|                                     |
|                                     |
|                                     |
|                 +-------------------+
|                 |                   |
|                 |    Layer 0        |
|                 |                   |
|                 |    blue           |
|                 |                   |
+-----------------+-------------------+

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
    #define USE_MIPI_PANEL MIPI_PANEL_RK055MHD091
    to
    #define USE_MIPI_PANEL MIPI_PANEL_RK055IQH091
    or
    #define USE_MIPI_PANEL MIPI_PANEL_RK055AHD091
    or
    #define USE_MIPI_PANEL DEMO_PANEL_RASPI_7INCH
    in lcdifv2_support.h.
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs, the screen shows what described in overview.
