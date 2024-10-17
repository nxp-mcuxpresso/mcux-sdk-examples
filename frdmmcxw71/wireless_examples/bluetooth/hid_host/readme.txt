Overview
========
This application implements a GATT based HID Profile.
This demo requires a serial port terminal application through a serial interface.
For more information, please consult the "BLE Demo Applications User's Guide".

Toolchain supported
===================
- IAR embedded Workbench (ide version details are in the Release Notes)
- MCUXpresso IDE (ide version details are in the Release Notes)
- GCC ARM Embedded (ide version details are in the Release Notes)

Hardware requirements
=====================
- Mini/micro USB cable
- frdmmcxw71 board

Board settings
==============
No special board setting.

Prepare the Demo
================
1.  Connect a mini/micro USB cable between the PC host and the OpenSDA USB port on the board.
2.  Download the program to the target board.
3.   Open a serial terminal on PC for the detected serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Press the reset button on your board to begin running the demo.

Running the demo
================
Follow the onscreen menu and instructions received over the serial interface.
