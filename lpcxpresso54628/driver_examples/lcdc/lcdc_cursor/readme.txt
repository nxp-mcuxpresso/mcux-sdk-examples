Overview
========
The lcdc_cursor example shows how to use LCD hardware cursor.

In this example, a 32x32 cursor is shown. The cursor's position is changed at
the end of every frame.

The background is:
+------------------------------------------------+
+                                                +
+                    Red                         +
+                                                +
+            +--------------------+              +
+            +                    +              +
+            +      Blank         +              +
+            +                    +              +
+            +--------------------+              +
+                                                +
+                                                +
+                                                +
+------------------------------------------------+

Toolchain supported
===================
- MCUXpresso  11.6.0
- GCC ARM Embedded  10.3.1

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
There is a cursor moves in the panel if the example runs successfully.
