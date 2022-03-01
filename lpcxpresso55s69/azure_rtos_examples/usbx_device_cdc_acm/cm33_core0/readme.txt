Overview
========
This example works as a USB CDC ACM device. It will appear as a USB serial device on PC.


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
6.  Press the on-board RESET button to start the demo. A new USB serial device will appear on PC.
7.  Open a serial terminal with the following settings for the new USB serial device.
    - 115200 baud rate
    - 8 data bits
    - No parity
    - 1 stop bit
    - No flow control

Running the demo
================
When the demo is running, the serial port of the Debug Link will output:

Start USBX device CDC ACM example...

Then, connect a USB cable between PC and the USB device port of the board.
A USB serial device will appear in the Device Manager of Windows.
And the serial port will output:

CDC device activate

In another serial terminal, press any key, and it will display a string, for example:

fabcdef
gabcdef
3abcdef

