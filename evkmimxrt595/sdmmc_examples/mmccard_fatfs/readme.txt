Overview
========
The MMCCARD FATFS project is a demonstration program that uses the SDK software. It mounts a file 
system based on a MMC card then does "creat directory/read directory/create file/write file/read 
file" operation. The file sdhc_config.h has default SDHC configuration which can be adjusted to let
card driver has different performance. The purpose of this example is to show how to use MMCCARD 
driver based FATFS disk in SDK software.

Note:
User can use MMC plus card or emmc(on board IC, but not recommand use emmc socket,due to high speed timing restriction)

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.7.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT595 REV C1 board
- Personal Computer

Board settings
==============
Make sure resistors R691~R697 are removed and resistors R611~R620,R660,R661,R698 are populated.


Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J40) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs successfully, the log would be seen on the terminal like:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FATFS example to demonstrate how to use FATFS with MMC card.



Make file system......The time may be long if the card capacity is big.



Create directory......



Create a file in that directory......



List the file in that directory......

General file : F_1.DAT.



 Write/read file until encounters error......



Write to above created file.

Read from above created file.

Compare the read/write content.

The read/write content is consistent.



Input 'q' to quit read/write.

Input other char to read/write file again.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
