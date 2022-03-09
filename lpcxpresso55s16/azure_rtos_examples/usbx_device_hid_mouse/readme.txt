Overview
========
This example works as a USB HID device. It will appear as a USB mouse device on PC.


Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Two Micro USB Cables
- Target Board
- Personal Computer(PC)

Board settings
==============
This example only work with the USB high speed port (J4).

Prepare the Demo
================
1.  Connect a USB Micro cable between the host PC and the Debug Link USB port (J1) on the target board.
2.  Connect a USB Micro cable between the host PC and the on-board USB high speed port (J4).
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - 1 stop bit
    - No flow control
3.  Compile the demo.
5.  Download the program to the target board.
6.  Press the on-board RESET button to start the demo. A new USB keyboard will appear on PC.
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
