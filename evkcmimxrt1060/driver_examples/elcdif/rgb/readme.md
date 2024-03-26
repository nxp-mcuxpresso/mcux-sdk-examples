Overview
========
The ELCDIF RGB project shows how to drive the RGB interface LCD using eLCDIF driver.
If this example runs correctly, a rectangle is moving in the screen, and the color
changes every time it reaches the edges of the screen.

The rectangle moving speed might be different with Debug target and Release target,
because these two targets spend different time to fill the frame buffer.

SDK version
===========
- Version: 2.15.100

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- MIMXRT1060-EVKC board
- Personal Computer
- RK043FN02H-CT or RK043FN66HS-CT6 LCD board
  (RK043FN02H-CT and RK043FN66HS-CT6 are compatible)

Board settings
==============
1. Connect the RK043FN02H-CT or RK043FN66HS-CT6 to board.

Prepare the Demo
================
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
If this example runs correctly, a rectangle is moving in the screen, and the color
changes every time it reaches the edges of the screen.
