Overview
========
This example can work with a USB HID mouse. When connecting
a USB HID mouse and pressing keys, the serial port will output
which key has been pressed.


Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- USB A to micro USB cable
- Target board
- Personal Computer
- USB mouse

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
    - 1 stop bit
    - No flow control
3.  Compile the demo with the configuration, "flexspi_nor_debug".
4.  Write the program to the flash of the target board.
5.  Press the reset button on your board to start the demo.
6.  Connect a USB mouse to the board.
7.  The serial terminal will dump mouse movement.
Running the demo
================
After writing the program to the flash of the target board,
press the reset button on your board to start the demo.
The serial port will output:

USBX host HID mouse example

Then, connect a USB mouse to the USB high speed device port of the board.
The example will print the pressed mouse movement.
