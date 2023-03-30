Overview
========
The example creates a simple washing machine demonstration. It shows
a UI on a LCD panel with touch panel. It also can interact with user.

Define GUIX_PXP_ENABLE to enable Pixel Pipeline engine (PXP).
It can accelerate the image process.


Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.7.0

Hardware requirements
=====================
- Micro USB cable
- EVKB-MIMXRT1060 board
- Personal Computer
- RK043FN02H-CT or RK043FN66HS-CT6 LCD board
  (RK043FN02H-CT and RK043FN66HS-CT6 are compatible)

Board settings
==============
1. Connect the LCD board to the on-board LCD connector J8.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Build the project. Note that the project uses the LCD board RK043FN66HS-CT6 by default.
    To use the LCD board RK043FN02H-CT, change the following line in display_support.h from
      #define DEMO_PANEL DEMO_PANEL_RK043FN66HS
    to
      #define DEMO_PANEL DEMO_PANEL_RK043FN02H
4.  Write the program to the flash of the target board.
5.  Press the reset button on your board to start the demo.

Running the demo
================
When the demo is running, a washing machine UI will display on the LCD panel. User can
touch the panel to change the temperature, water level, ...

The serial port will output:
Start the GUIX washing machine example...

