Overview
========
The Localization User Device acts as a Bluetooth Low Energy central that implements the Ranging Service (RAS) server side. Both the initiator and reflector Channel Sounding roles are supported and may be set at compile time. In the default configuration the initiator role is used. The RAS server stores local measurement data and sends it to the RAS client. The Localization User Device does not run the localization algorithms.
For more information, please consult AN13974 "Bluetooth Low Energy Localization Application Note".

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
3.  Press the reset button on your board to begin running the demo.

Running the demo
================
Please consult the "Bluetooth Low Energy Localization Application Note".
