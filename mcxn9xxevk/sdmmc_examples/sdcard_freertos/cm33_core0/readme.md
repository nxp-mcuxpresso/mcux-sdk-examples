Overview
========
The SDCARD FreeRTOS project is a demonstration program that uses the SDK software. It reads/writes
/erases the SD card continusly. The purpose of this example is to show how to use SDCARD driver 
with FreeRTOS in SDK software to access SD card.
Note: If DATA3 is used as the card detect PIN, please make sure DATA3 is pull down, no matter internal or external, at the same time, make sure the card can pull DATA3 up, then host can detect card through DATA3.And SDHC do not support detect card through CD by host, card can be detected through DATA3 or GPIO.No matter detect card through host or gpio, make sure the pinmux configuration is correct.


SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- MCX-N9XX-EVK board
- Personal Computer
- SD card

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J5) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.
Please insert the SDCARD into card slot(J25)

Running the demo
================
When the demo runs successfully, the log would be seen on the terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

SDCARD freertos example.

Card inserted.

Read/Write/Erase the card continuously until encounter error......

Write/read one data block......
Compare the read/write content......
The read/write content is consistent.
Write/read multiple data blocks......
Compare the read/write content......
The read/write content is consistent.
Erase multiple data blocks......

Input 'q' to quit card access task.
Input other char to read/write/erase data blocks again.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If sd card is not inserted before example run, then more log will generated like:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SDCARD freertos example.

Please insert a card into board.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
