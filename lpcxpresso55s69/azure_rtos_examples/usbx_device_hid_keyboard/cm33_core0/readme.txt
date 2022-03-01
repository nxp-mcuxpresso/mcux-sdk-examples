Overview
========
This example works as a USB HID device. It will appear as a USB keyboard device on PC.


Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Two Micro USB cables
- Target Board
- Personal Computer(PC)

Board settings
==============
This example only work with the USB high speed port (P9).

Prepare the Demo
================
1.  Connect a USB Micro cable between the host PC and the Debug Link USB port (P6) on the target board.
2.  Connect a USB Micro cable between the host PC and the on-board USB high speed port (P9).
3.  Open a serial terminal on PC with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - 1 stop bit
    - No flow control
4.  Compile the demo.
5.  Download the program to the target board.
6.  Press the on-board RESET button to start the demo.

Running the demo
================
After writing the program to the flash of the target board,
press the reset button on your board to start the demo.
The serial port will output:

USBX device HID Keyboard example

Then, connect a USB cable between PC and the USB device port
of the board. A HID-compliant keyboard will appear in the
Device Manager of Windows and send a key every 2 secondes.

The serial port will output:

HID device activate
