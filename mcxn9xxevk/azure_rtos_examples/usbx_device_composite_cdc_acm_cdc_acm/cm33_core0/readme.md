Overview
========
This example works as two USB CDC ACM devices. It will appear as two USB serial devices on PC.


SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

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
6.  PC can detect two USB ACM devices.

Running the demo
================
When the demo is running, the serial port of the Debug Link will output:

Start USBX device composite example...

Then, connect a USB cable between PC and the USB device port of the board.
Two USB serial devices will appear in the Device Manager of Windows.
And the serial port will output:

CDC ACM1 device activate
CDC ACM2 device activate

ACM1 device serial terminal, press any key, and it will display a string, for example:

fabcdef
gabcdef
3abcdef

ACM2 device serial terminal, press any key, and it will display a string, for example:

f123456
g123456
3123456
