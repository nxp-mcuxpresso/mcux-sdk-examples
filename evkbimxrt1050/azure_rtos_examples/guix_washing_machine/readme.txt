Overview
========
The example creates a simple washing machine demonstration. It shows
a UI on a LCD panel with touch panel. It also can interact with user.


Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Micro USB cable
- EVKB-IMXRT1050 board
- Personal Computer
- RK043FN02H-CT LCD board

Board settings
==============
1. Connect the RK043FN02H-CT LCD board to J8.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Write the program to the flash of the target board.
4.  Press the reset button on your board to start the demo.

Running the demo
================
When the demo is running, a washing machine UI will display on the LCD panel. User can
touch the panel to change the temperature, water level, ...

The serial port will output:
Start the GUIX washing machine example...

