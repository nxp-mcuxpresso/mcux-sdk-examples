Overview
========
The example shows how to use FileX and LevelX with the mflash component.

This example will erase the SPI flash and format it in FAT format.
Then, do some file operation test - create, write, read, close and so on.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- Target Board
- Personal Computer

Board settings
==============
For the rev A board:
Connect pin 1-2 of JP41,JP42,JP43,JP44,JP45,JP61,JP62,JP64 and JP65.
Close JP66,JP67,JP68 and JP69.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the LPC-Link USB port (J1) on the board.
2.  Open a serial terminal on PC with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Compile the demo.
4.  Download the program to the target board.
5.  Press the on-board RESET button to start the demo.

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

