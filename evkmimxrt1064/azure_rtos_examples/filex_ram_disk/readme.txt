Overview
========
This is a small demo of the high-performance FileX FAT file system.
It includes setup for a small 34KB RAM disk and a loop that writes
and reads a small file.


Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Mini/micro USB cable
- EVK-MIMXRT1064 board
- Personal Computer

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
3.  Compile the demo with the configuration, "flexspi_nor_debug".
4.  Write the program to the flash of the target board.
5.  Press the reset button on your board to start the demo.

Running the demo
================
After writing the program to the flash of the target board,
press the reset button on your board to start the demo.

Example output:
FILEX example.....
Open RAM Disk.....
Creat TEST.TXT.....
Write TEST.TXT.....
Close TEST.TXT.....
Close RAM Disk.....
