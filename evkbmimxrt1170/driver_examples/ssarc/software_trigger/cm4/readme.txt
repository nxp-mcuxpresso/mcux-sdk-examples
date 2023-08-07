Overview
========
The SSARC Software Trigger example show how to config SSARC to software-trigger the save/restore operation
in RUN Mode.

The sequence of the example is followed. Turn the LED, then save the contains of GPIO's DATA Register.
Turn off the LED, then restore the saved register. Finally, you will see the LED reopened.

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVKB board
- Personal Computer

Board settings
==============
No special settings are required.

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
SSARC Software Trigger Example Start.
Open the LED.
Please press any key to trigger save operation.
Saving. Please Wait.
Closed the LED.
Please press any key to trigger restore operation. After restoring, the LED will be re-opened.
Restoring. Please Wait.
SSARC Software Trigger Example Finish.
