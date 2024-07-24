Overview
========
The application implements a Bluetooth Low Energy Host with a shell inteface.
For more information, please consult the "BLE Demo Applications User's Guide".

Toolchain supported
===================
- IAR embedded Workbench (ide version details are in the Release Notes)
- MCUXpresso IDE (ide version details are in the Release Notes)

Hardware requirements
=====================
- Mini/micro USB cable
- frdmmcxw71 board

Board settings
==============
No special board setting.

Prepare the Demo
================
<<<<<<<< HEAD:boards/frdmmcxw71/wireless_examples/bluetooth/ble_shell/readme.md
1. Connect a mini/micro USB cable between the PC host and the OpenSDA USB port on the board.
2. Download the program to the target board.
3. Press the reset button on your board to begin running the demo.
4. Open a serial terminal application and use the following settings with the detected serial device:
   - 115200 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control
========
1.  Connect a mini/micro USB cable between the PC host and the OpenSDA USB port on the board.
3.  Download the program to the target board.
4.  Press the reset button on your board to begin running the demo.
5.  Open a serial terminal application and use the following settings with the detected serial device:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
>>>>>>>> parent of eac40fa683 (Pull request #3081: [BLUETOOTH-13752][KW45]SDK2.16 - readme.txt-readme.md):boards/frdmmcxw71/wireless_examples/bluetooth/ble_shell/readme.txt

Running the demo
================
Use a serial terminal to interact with the device.
