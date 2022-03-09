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

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Mini/micro USB cable
- EVK-MIMXRT1160 board
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
When the demo runs successfully, the screen shows as the overview descripted.
