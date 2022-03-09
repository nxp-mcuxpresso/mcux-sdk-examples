Overview
========
The touch_cursor demo shows how to use LCD hardware cursor and position it using touch panel.

In this example, a 32x32 cursor is shown. The cursor's position is changed in reaction to detected touch.

Toolchain supported
===================
- MCUXpresso  11.5.0
- GCC ARM Embedded  10.2.1

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso54628 board
- Personal Computer

Board settings
==============
No special board settings.

Prepare the Demo
================
1. Connect a micro USB cable between the PC host and the LPC-Link USB port (J8) on the board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running
   the demo.

Running the demo
================
If the example runs successfully, there is a 32x32 cursor shows in panel and moves in the panel with the movement of the detected touch, the coordinate of the cursor will also show in the terminal.
