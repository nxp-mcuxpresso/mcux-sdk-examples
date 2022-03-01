Overview
========
This example illustrates USBX Device Mass Storage.


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

USBX device mass storage example

Then, connect a USB cable between PC and USB device port
of the board. The serial port will output:

USB MSD device activate

PC will detect a u-disk and can format it. After format
is completed, the PC will display a removable disk and
it can be used as a normal u-disk.

Please note that the USBX Device Mass Storage example use
RAM disk as storage media, data will lost after board is
reset.
