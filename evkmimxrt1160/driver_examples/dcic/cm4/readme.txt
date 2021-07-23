Overview
========

This example shows how to use DCIC to monitor the display content integrity.
In this example, DCIC monitors part of the screen, user could press input
through the debug terminal, to let the screen show two types of frames,
one matches the reference CRC (right frame), the other does not match (wrong frame).
User could measure the output signal pin, when the wrong frame is shown, the
output signal frequency is 1/4 of the frequency when right frame is shown.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.4.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1160-EVK board
- Personal Computer
- RK055AHD091 panel or RK055IQH091 panel

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
3.  Build the project, the project uses RK055AHD091 by default, to use panel RK055IQH091,
    change
    #define DEMO_PANEL DEMO_PANEL_RK055AHD091
    to
    #define DEMO_PANEL DEMO_PANEL_RK055IQH091
    in display_support.h.
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs, press any key in terminal to switch display frame, and
measure the J10-12 using oscilloscope.
