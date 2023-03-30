Overview
========
This project shows how to use the CSI with MIPI_CSI. In this example, the
camera device output pixel format is RGB565, the MIPI_CSI converts it to
RGB888 internally and sends to CSI. In other words, the CSI input data bus
width is 24-bit. The CSI saves the frame as 32-bit format XRGB8888. PXP
is used to convert the XRGB8888 to RGB565 and shown in the LCD panel.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.7.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVK board
- Personal Computer
- RK055AHD091 panel or RK055MHD091 panel
- OV5640 camera

Board settings
==============
Connect the panel to J48
Connect camera to J2
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
3.  Build the project, this project uses RK055MHD091 panel by default, to use the RK055AHD091 panel,
    change #define DEMO_PANEL DEMO_PANEL_RK055MHD091 to #define DEMO_PANEL DEMO_PANEL_RK055AHD091
    in display_support.h.
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs, the screen shows the camera input frame.
