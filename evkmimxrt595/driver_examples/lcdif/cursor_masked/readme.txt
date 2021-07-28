Overview
========
This example demonstrates how to use the LCDIF cursor in masked mode.
In this exapmle, the screen is devided into two parts: red and blue. A cursor
is moving in the screen, the cursor contains 4 parts like this:

          +-----------------------+
          |        Part 1         |
          +-----------------------+
          |        Part 2         |
          +-----------------------+
          |        Part 3         |
          +-----------------------+
          |        Part 4         |
          +-----------------------+

Part 1 is background color set by LCDIF_SetCursorColor, it is black.
Part 2 is the backgroud frame buffer color.
Part 3 is foreground color set by LCDIF_SetCursorColor, it is white.
Part 4 is the invertion of backgroud frame buffer color.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.4.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT595 board
- Personal Computer
- RK055AHD091 panel

Board settings
==============
Connect the RK055AHD091 panel to EVK-MIMXRT595 board J44.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J40) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs, the screen shows what described in overview.
