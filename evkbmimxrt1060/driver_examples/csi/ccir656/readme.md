Overview
========
The CSI CCIR656 project shows how to receive the camera data using CSI driver,
the camera interface is CCIR656. In this example, you will see the camera input
image shown in the LCD. Please note that the camera input image resolution
might be smaller than the LCD panel, so part of the LCD panel might be blank.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1060-EVKB board
- Personal Computer
- RK043FN02H-CT or RK043FN66HS-CT6 LCD board
  (RK043FN02H-CT and RK043FN66HS-CT6 are compatible)
- MT9M114 module
This example supports OV7725 too, if OV7725 is used, then MT9M114 is not necessary.

Board settings
==============
1. Connect the RK043FN02H-CT or RK043FN66HS-CT6 to board.
2. Connect the MT9M114 or OV7725 camera module.

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

Note: If OV7725 is used, change the macro DEMO_CAMERA to DEMO_CAMERA_OV7725 in
camera_support.h

Running the demo
================
When the demo runs successfully, the camera received pictures are shown in the LCD.
