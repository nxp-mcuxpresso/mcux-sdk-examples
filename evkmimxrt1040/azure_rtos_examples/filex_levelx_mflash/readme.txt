Overview
========
The example shows how to use FileX and LevelX with the mflash component.

This example will erase the SPI flash and format it in FAT format.
Then, do some file operation test - create, write, read, close and so on.


Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Mini/micro USB cable
- EVK-MIMXRT1040 board
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
When the example runs successfully, the console will output like:

Start FileX LevelX SPI Flash example
Erase Flash: offset = 0x0, size = 512 KB
................................................................................................................................
Fromat: disk_size = 480 KB

Creat TEST.TXT
Open TEST.TXT
Write TEST.TXT
Read TEST.TXT
Close TEST.TXT

Continue the test (y/n):

