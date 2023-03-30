Overview
========
This example can work with a USB HID keyboard. When connecting
a USB HID keyboard and pressing keys, the serial port will output
which key has been pressed.


Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.7.0

Hardware requirements
=====================
- A Micro USB cables
- One USB A Female to Micro B Male cable
- Target Board
- Personal Computer(PC)
- A USB keyboard device

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
3.  Compile the demo.
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.
6.  Connect a USB keyboard device to the USB1 port of the board via a USB A Female to Micro B Male cable.
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
