Overview
========
This application implements a Beacon BLE Service.
To test the service/profile the NXP IoT Toolbox application can be used which is available for both Android and iOS.
For more information, please consult the "BLE Demo Applications User's Guide".

Toolchain supported
===================
- IAR embedded Workbench (ide version details are in the Release Notes)
- MCUXpresso IDE (ide version details are in the Release Notes)

Hardware requirements
=====================
- Mini/micro USB cable
- k32w148evk board

Board settings
==============
No special board setting.
To flash the board in case the sensor is put in deep sleep, press SW3 or RESET.

Prepare the Demo
================
1.  Connect a mini/micro USB cable between the PC host and the OpenSDA USB port on the board.
2.  Download the program to the target board.
3.  Press the reset button and ADVSW on your board to begin running the demo.

Running the demo
================
Interact with the device using the NXP IoT Toolbox or your custom application which supports the implemented BLE profile(s)
