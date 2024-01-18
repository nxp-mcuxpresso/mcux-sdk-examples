Overview
========
The PXP color key project shows how to use the AS color key together with the
alpha blend. In this example, the AS pixel format is RGB565, the global alpha
is used for alpha blend.

The PS frame buffer is like this:

+---------------------------------+-----------------------------------+
|                                 |                                   |
|                                 |                                   |
|                                 |                                   |
|                                 |                                   |
|                                 |                                   |
|                                 |                                   |
|                                 |                                   |
|           BLUE                  |             RED                   |
|                                 |                                   |
|                                 |                                   |
|                                 |                                   |
|                                 |                                   |
|                                 |                                   |
|                                 |                                   |
|                                 |                                   |
|                                 |                                   |
|                                 |                                   |
+---------------------------------+-----------------------------------+

The AS frame buffer is square like this:

               +---------------------------------+
               |                                 |
               |                                 |
               |             GREEN               |
               |                                 |
               |        +---------------+        |
               |        |               |        |
               |        |               |        |
               |        |               |        |
               |        |    YELLOW     |        |
               |        |               |        |
               |        |               |        |
               |        |               |        |
               |        +---------------+        |
               |                                 |
               |                                 |
               |                                 |
               +---------------------------------+

The AS is placed in the center of the screen. The AS color key is set to yellow
color, the blend alpha value is set to 128. So in the output frame buffer, the
originally yellow region shows PS color, the green region is blend with PS color.
The screen shows the output frame like this:

+-------------+-------------------+-------------------+---------------+
|             |                   |                   |               |
|             |   HALF CYAN       |    HALF YELLOW    |               |
|             |                   |                   |               |
|             |                   |                   |               |
|             |          +--------+---------+         |               |
|             |          |        |         |         |               |
|             |          |        |         |         |               |
|   BLUE      |          | BLUE   |  RED    |         |    RED        |
|             |          |        |         |         |               |
|             |          |        |         |         |               |
|             |          |        |         |         |               |
|             |          +--------+---------+         |               |
|             |                   |                   |               |
|             |                   |                   |               |
|             |                   |                   |               |
|             |                   |                   |               |
|             |                   |                   |               |
+-------------+-------------------+-------------------+---------------+

SDK version
===========
- Version: 2.15.0

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
When the demo runs successfully, the screen shows as the overview descripted.
