Overview
========
This example illustrates USBX Device Mass Storage.


SDK version
===========
- Version: 2.15.100

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
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
    - 1 stop bit
    - No flow control
3.  Compile the demo with the configuration, "flexspi_nor_debug".
4.  Write the program to the flash of the target board.
5.  Press the reset button on your board to start the demo.
6.  Connect a USB cable between the PC and the USB device port of the board.
7.  PC can format/read/write this u-disk.
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
