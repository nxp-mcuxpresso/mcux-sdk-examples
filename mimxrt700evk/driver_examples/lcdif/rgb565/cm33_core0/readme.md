Overview
========
This example demonstrates how to use the LCDIF driver to show the RGB565 format
frame buffer. When the example runs, a rectangle is moving in the screen, and
its color changes when touch border.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- MIMXRT700-EVK
- Personal Computer
- RK055AHD091 MIPI panel (Not necessary if use other panel. Not used in lcdif_dbi_rgb565 project)
- RK055MHD091 MIPI panel (Not necessary if use other panel. Not used in lcdif_dbi_rgb565 project)
- TFT Proto 5" CAPACITIVE board HW REV 1.01 by Mikroelektronika. (Named as SSD1963 panel in project. Not necessary if use MIPI panel. Only used in lcdif_dbi_rgb565 project.)
- RM67162 smart MIPI panel (Not necessary if use other panel. Only used in lcdif_dbi_rgb565 project.)
- RaspberryPi Panel (Not necessary if use other panel. Not used in lcdif_dbi_rgb565 project)

Board settings
==============
To use MIPI panel:
Connect MIPI panel to J52.

To use RaspberryPi panel:
Connect the panel to J8. Then connect the panel's 5V pin to JP43-1, GND pin to JP43-2.
Make sure the R75, R76, R79, R80 are connected.

To use SSD1963 panel:
Connect SSD1963 panel to J4. Make sure to connect JP7 2&3 to use 3.3v interface.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J54) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Build the project, to specify which panel to use, change the macro USE_DBI_PANEL(only example lcdif_dbi_rgb565)
    or USE_MIPI_PANEL(except example lcdif_dbi_rgb565) in lcdif_support.h
4.  Download the program to the target board.
5.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs, the screen shows what described in overview.
