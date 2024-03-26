Overview
========
This example illustrates USBX Host Mass Storage.


SDK version
===========
- Version: 2.15.100

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- A Micro USB cables
- One USB A Female to Micro B Male cable
- Target Board
- Personal Computer(PC)
- A U-disk device

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
6.  Connect a U-disk device to the USB1 port of the board via a USB A Female to Micro B Male cable.

Running the demo
================
After writing the program to the flash of the target board,
press the reset button on your board to start the demo.
The serial port will output:

USBX host mass storage example

Then, connect a U-disk to the USB device port of the board.
The example will display the vendor ID and the product ID of
the attached USB device, then scan the disk and dump the file
name to the serial port.

For example:

USB device: vid=0x2ce3, pid=0x6487
Find Dir: DIR_1
Find Dir: DIR_2
Find File: 1234.text
Find File: test.text

