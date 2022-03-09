Overview
========
This example shows how to use the fbdev component. The fbdev masks
the difference of LCD controllers and provides a unified APIs for upper
layers. At the same time, the frame buffers are managed by the FBDEV.
When the example runs, you can see a rectangle moving in the screen, and
its color changes when reached the border.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Micro USB cable
- TFT Proto 5" CAPACITIVE board HW REV 1.01 by Mikroelektronika. (Named as SSD1963 panel in project. Not necessary if use MIPI panel)
- EVK-MIMXRT595
- Personal Computer
- RK055AHD091 MIPI panel (Not necessary if use other panel)
- RK055MHD091 MIPI panel (Not necessary if use other panel)
- RM67162 smart MIPI panel (Not necessary if use other panel)

Board settings
==============
To use SSD1963 panel:
Connect SSD1963 panel to J43.

To use MIPI panel:
Connect MIPI panel to J44.

Prepare the Demo
================
The demo use SSD1963 panel by default, to use MIPI RK055AHD091 panel, change the macro
DEMO_PANEL to DEMO_PANEL_RK055AHD091 in display_support.h. To use RM67162 MIPI
smart panel, change the macro DEMO_PANEL to DEMO_PANEL_RM67162 in display_support.h.
To use MIPI RK055MHD091 panel, change the macro DEMO_PANEL to DEMO_PANEL_RK055AHD091
in display_support.h.

1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs, you can see a rectangle moving in the screen, and
its color changes when reached the border.
There is tearing effect when using the SSD1963 panel, because the panel video buffer
could not be fully updated during fresh interval.
