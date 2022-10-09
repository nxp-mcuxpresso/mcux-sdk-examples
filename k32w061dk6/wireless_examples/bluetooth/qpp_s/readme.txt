Overview
========
This application implements a GATT based Private Profile (QPP - Quintic Private Profile).
For more information, please consult the "BLE Demo Applications User's Guide".

Toolchain supported
===================
- IAR Embedded Workbench
- MCUXpresso IDE

Hardware requirements
=====================
- Mini/micro USB cable
- QN9090/K32W board

Board settings
==============
No special board setting.

Prepare the Demo
================
1.  Connect a mini/micro USB cable between the PC host and the OpenSDA USB port on the board.
2.  Download the program to the target board.
3.  Press the reset button on your board to begin running the demo.

Running the demo
================
Interact with the device using another device running the Private Profile Client.

Note
================
Now, the QPP_S supports NTAG feature, to enable this feature:
- Set the macro gAppNtagSupported_d to '1' in app_preinclude.h of QPP_S.
- Press the ADVSW button of device, run 'IOT Toolbox' and connect QPP_S board.
- Then, disconnect QPP_S, go to main menu of IoT Toolbox and run NFC test utility.
- Next, put your phone close to the NFC coil of board, and press "Read device Name" button of NFC utility.
- Finally, you can get the device name of the board via NFC, it will be shown on the screen. 
