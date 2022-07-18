Overview
========
This project shows how to receive the camera RAW8 data using CSI driver.
In this example, the RAW8 data is convert to RGB565 data, then shown in the panel.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1060-EVKB board
- Personal Computer
- RK043FN02H-CT or RK043FN66HS-CT6 LCD board
  (RK043FN02H-CT and RK043FN66HS-CT6 are compatible)
- MT9M114 module

Board settings
==============
1. Connect the RK043FN02H-CT or RK043FN66HS-CT6 to board.
2. Connect the MT9M114 camera module.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Launch the debugger in your IDE to begin running the demo.

Note: This project only support MT9M114.

Running the demo
================
When the demo runs successfully, the camera received pictures are shown in the LCD.
