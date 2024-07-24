Overview
========
The MMCCARD Polling project is a demonstration program that uses the SDK software. It reads/writes
/erases the MMC card continusly. The purpose of this example is to show how to use MMCCARD driver and
show how to use polling based transfer API in SDK software driver to access MMC card.

Note:
User can use MMC plus card or emmc(on board IC, but not recommand use emmc socket,due to high speed timing restriction)

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso54628 board
- Personal Computer

Board settings
==============
Insert the card into card slot

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the LPC-Link USB port (J8) on the board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Reset the SoC and run the project.

Running the demo
================
When the demo runs successfully, the log would be seen on the terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

MMCCARD polling example.

Card user partition size xxxx bytes

Working condition:

  Voltage: VCC - xxxx  VCCQ - xxxx

  Timing mode: High Speed

  Bus width: xxxx

  Freq : xxxx HZ

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
