Overview
========
The application implements a custom GATT based Wireless UART Profile that emulates UART over BLE.
To test the service/profile the IoT Toolbox application can be used which is available for both Android and iOS.
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
4.  Open a serial terminal application and use the following settings with the the detected serial device:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control

Running the demo
================
Use a serial terminal to interact with the device.
