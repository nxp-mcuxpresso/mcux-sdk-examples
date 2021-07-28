Overview
========
The MMCCARD Polling project is a demonstration program that uses the SDK software. It reads/writes
/erases the MMC card continusly. The purpose of this example is to show how to use MMCCARD driver and
show how to use polling based transfer API in SDK software driver to access MMC card.

Note:
User can use MMC plus card or emmc(on board IC, but not recommand use emmc socket,due to high speed timing restriction)

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.4.0

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
Please insert the SDCARD into card slot(J32)

Running the demo
================
When the demo runs successfully, the log would be seen on the terminal like:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MMCCARD polling example.



Card user partition size xxxx bytes



Working condition:



  Voltage: xxxx



  Timing mode: xxxx



  Bus width: xxxx



  Freq : xxxx



Read/Write the card continuously until encounter error....



Write/read one data block......

Compare the read/write content......

The read/write content is consistent.

Write/read multiple data blocks......

Compare the read/write content......

The read/write content is consistent.

Erase data groups......



Input 'q' to quit read/write/erase process.

Input other char to read/write/erase data blocks again.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
