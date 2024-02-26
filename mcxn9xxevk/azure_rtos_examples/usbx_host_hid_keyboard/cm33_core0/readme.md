Overview
========
This example can work with a USB HID keyboard. When connecting
a USB HID keyboard and pressing keys, the serial port will output
which key has been pressed.


SDK version
===========
- Version: 2.14.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Mini/micro USB cable
- USB A to micro USB cable
- Target board
- Personal Computer
- USB keyboard

Board settings
==============
This example only supports the high-speed USB port, J27.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J5) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - 1 stop bit
    - No flow control
3.  Write the program to the flash of the target board.
4.  Press the reset button on your board to start the demo.
5.  Connect a USB keyboard to the on-board high-speed USB port, J27.
6.  The serial terminal will dump keyboard input.

Running the demo
================
After writing the program to the flash of the target board,
press the reset button on your board to start the demo.
The serial port will output:

USBX host HID Keyboard example

Then, connect a USB Keyboard to the USB high speed device port of the board.
The example will print the pressed keyboard key.

For example:
Input: a
Input: b
Input: c
