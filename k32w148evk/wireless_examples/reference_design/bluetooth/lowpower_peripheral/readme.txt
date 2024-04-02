Overview
========
This application is based on a GATT Temperature Service and demonstrates power consumption optimization in BLE.
Power consumption is optimized during advertising, connected and no activity states.
For more information, please consult the "Low Power Connectivity Design User's Guide".

Toolchain supported
===================
- IAR Embedded Workbench
- MCUXpresso IDE

Hardware requirements
=====================
- Mini/micro USB cable
- frdmkw38 board

Board settings
==============
No special board setting.

Prepare the Demo
================
1.  Connect a mini/micro USB cable between the PC host and the OpenSDA USB port on the board.
3.  Download the program to the target board.
4.  Press the reset button on your board to begin running the demo.

Running the demo
================
At power on, device is advertising. A device with Temperature Collector application could be used to get connected.

