Overview
========
This example works as a USB HID device. It will appear as a USB mouse device on PC.


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
- Target Board
- Personal Computer(PC)

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
5.  Connect a USB cable between the PC and the on-board high-speed USB port, J27.

Running the demo
================
After writing the program to the flash of the target board,
press the reset button on your board to start the demo.
The serial port will output:

USBX device HID mouse example

Then, connect a USB cable between PC and the USB device port
of the board. A HID-compliant mouse will appear in the
Device Manager of Windows and draw a rectangle.

The serial port will output:

HID device activate
