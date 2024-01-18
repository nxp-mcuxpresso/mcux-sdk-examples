Overview
========
The touch_cursor demo shows how to use LCD hardware cursor and position it using touch panel.

In this example, a 32x32 cursor is shown. The cursor's position is changed in reaction to detected touch.

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso54628 board
- RK043FN02H-CT or RK043FN66HS-CT6 LCD board
- Personal Computer

Board settings
==============
1. Connect the RK043FN02H-CT or RK043FN66HS-CT6 to board.

Prepare the Demo
================
1. Connect a micro USB cable between the PC host and the LPC-Link USB port (J8) on the board.
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
    in touch_cursor.c
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
If the example runs successfully, there is a 32x32 cursor shows in panel and moves in the panel with the movement of the detected touch, the coordinate of the cursor will also show in the terminal.
