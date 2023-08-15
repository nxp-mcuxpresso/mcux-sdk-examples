Overview
========
The SDCARD Polling project is a demonstration program that uses the SDK software. It reads/writes
/erases the SD card continusly. The purpose of this example is to show how to use SDCARD driver and
show how to use polling based transfer API in SDHC driver in SDK software to access SD card.
Note: If DATA3 is used as the card detect PIN, please make sure DATA3 is pull down, no matter internal or external, at the same time, make sure the card can pull DATA3 up, then host can detect card through DATA3.And SDHC do not support detect card through CD by host, card can be detected through DATA3 or GPIO.No matter detect card through host or gpio, make sure the pinmux configuration is correct.

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- FRDM-K32L3A6 board
- Personal Computer
- micro SD card

Board settings
==============
1.Please insert the SDCARD into card slot(J9)
2.Make sure R153. R154, R156, R157 is switch to 1-2.
Note:
Due to FRDM board do not provide 1.8V IO voltage for SD part, so cannot switch to DDR50 mode.
And the CARD detect PIN level is high when card is inserted.

Prepare the Demo
================
1. Connect a USB cable between the PC host and the OpenSDA USB port on the board.
2. Open a serial terminal with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
When the demo runs successfully, the log would be seen on the terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Card size xxxxx bytes

Working condition:

  Voltage : xxxV

  Timing mode: xxx

  Freq : xxx HZ

Read/Write/Erase the card continuously until encounter error......

Write/read one data block......
Compare the read/write content......
The read/write content is consistent.
Write/read multiple data blocks......
Compare the read/write content......
The read/write content is consistent.
Erase multiple data blocks......

Input 'q' to quit read/write/erase process.
Input other char to read/write/erase data blocks again.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
