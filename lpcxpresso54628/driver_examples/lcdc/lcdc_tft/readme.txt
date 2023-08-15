Overview
========
The lcdc_tft example shows how to use LCD driver to drive TFT panel.

In this example, the cursor palette is used. A rectangle is shown in the panel,
its color and position are changed every frame.

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso54628 board
- Personal Computer
- RK043FN02H-CT or RK043FN66HS-CT6 LCD board

Board settings
==============
No special board settings.

Prepare the Demo
================
1. Connect a micro USB cable between the PC host and the LPC-Link USB port (J8) on the board.
2. Download the program to the target board.
3. Either press the reset button on your board or launch the debugger in your IDE to begin running
   the demo.

Running the demo
================
There is a rectangle moving smoothly and changing color when reach the edges.
