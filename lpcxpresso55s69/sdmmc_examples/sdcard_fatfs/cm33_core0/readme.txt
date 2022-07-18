Overview
========
The SDCARD FATFS project is a demonstration program that uses the SDK software. It mounts a file 
system based on a SD card then does "creat directory/read directory/create file/write file/read file"
operation. The file sdhc_config.h has default SDHC configuration which can be adjusted to let card
driver has different performance. The purpose of this example is to show how to use SDCARD driver 
based FATFS disk in SDK software.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Mini USB cable
- LPCXpresso55S69 board
- Personal Computer

Board settings
==============
Insert the card into the card slot

Prepare the Example
Note: MCUXpresso IDE project default debug console is semihost
1. Connect a micro USB cable between the PC host and the LPC-Link USB port (P6) on the board.
2. Open a serial terminal on PC for OpenSDA serial device with these settings:
   - 115200 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control
3. Download the program to the target board.
4. Reset the SoC and run the project.

Running the demo
================
When the demo runs successfully, the log would be seen on the terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

FATFS example to demonstrate how to use FATFS with SD card.

Please insert a card into board.

Card inserted.

Make file system......The time may be long if the card capacity is big.

Create directory......

Create a file in that directory......

Create a directory in that directory......

List the file in that directory......
General file : F_1.DAT.
Directory file : DIR_2.

Write/read file until encounters error......

Write to above created file.
Read from above created file.
Compare the read/write content......
The read/write content is consistent.

Input 'q' to quit read/write.
Input other char to read/write file again.

​~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
