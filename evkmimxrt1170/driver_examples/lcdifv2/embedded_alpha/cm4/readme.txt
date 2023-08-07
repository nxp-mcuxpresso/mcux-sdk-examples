Overview
========
This example demonstrates how to use the LCDIF v2 multiple layer blend using
the alpha embedded in pixel.
In this example, three layers are enabled. Layer 0 uses blue color RGB565 with
global alpha 50%. Layer 1 uses red color ARGB4444 with embedded_alpha %50.
Layer 2 uses green color ARGB8888 with embedded_alpha %25. The three layers are
all smaller than the screen size. By changing the layer offset, the color block
moves in the screen. The overlay region shows the blend color.

The initial screen is like this.

+------------------------------------+
|               |                    |
|  BLUE         |                    |
|       +-------+-------+            |
|       |PURPLE | RED   |            |
|       |       |       |            |
|-------+-------+-------+-------+    |
|       | RED   |YELLOW |       |    |
|       |       |       |       |    |
|       +-------+-------+       |    |
|               |               |    |
|               |     GREEN     |    |
|               +---------------+    |
|                                    |
|                                    |
|                                    |
+------------------------------------+

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVK board
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
    #define USE_MIPI_PANEL MIPI_PANEL_RK055MHD091
    to
    #define USE_MIPI_PANEL MIPI_PANEL_RK055IQH091
    or
    #define USE_MIPI_PANEL MIPI_PANEL_RK055AHD091
    in lcdifv2_support.h.
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs, the screen shows what described in overview.
