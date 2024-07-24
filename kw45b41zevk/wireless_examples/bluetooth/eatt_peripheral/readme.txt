Overview
========
This application implements 2 custom GATT based Services and sends data for each profile on an enhanced ATT bearer.
For more information, please consult the "BLE Demo Applications User's Guide".

Toolchain supported
===================
- IAR embedded Workbench (ide version details are in the Release Notes)
- MCUXpresso IDE (ide version details are in the Release Notes)

Hardware requirements
=====================
- Mini/micro USB cable
- kw45b41zevk board

Board settings
==============
No special board setting.
To flash the board in case the sensor is put in deep sleep, press SW3 or RESET.

Prepare the Demo
================
1.  Connect a mini/micro USB cable between the PC host and the OpenSDA USB port on the board.
2.  Download the program to the target board.
3.  Press the reset button on your board to begin running the demo.

Running the demo
================
Interact with the device using the custom EATT central application which supports the implemented BLE profile(s)
