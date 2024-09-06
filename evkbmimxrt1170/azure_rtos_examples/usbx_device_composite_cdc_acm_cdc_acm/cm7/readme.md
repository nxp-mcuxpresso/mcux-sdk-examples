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
- Two Micro USB cables
- Target Board
- Personal Computer(PC)

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
    - One stop bit
    - No flow control
3.  Compile the demo.
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.
6.  Connect a USB cable between the PC and the USB1 device port of the board.
7.  PC can detect the USB mouse device.
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
