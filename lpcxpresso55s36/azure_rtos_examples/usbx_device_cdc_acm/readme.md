Overview
========
This example works as a USB CDC ACM device. It will appear as a USB serial device on PC.


SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Two Micro USB cables
- Target Board
- Personal Computer(PC)

Board settings
==============
Set USB port J3 to device mode by opening jumper JP2.

Prepare the Demo
================
1.  Connect a USB Micro cable between the host PC and the Debug Link USB port (J1) on the target board.
2.  Connect a USB Micro cable between the host PC and the on-board USB full speed port (J3).
3.  Open a serial terminal on PC with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - 1 stop bit
    - No flow control
4.  Download the program to the target board.
5.  Press the on-board RESET button to start the demo.
6.  Open a serial terminal with the following settings for the new USB serial device.
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

