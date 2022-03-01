Overview
========
The SDCARD FreeRTOS project is a demonstration program that uses the SDK software. It reads/writes
/erases the SD card continusly. The purpose of this example is to show how to use SDCARD driver 
with FreeRTOS in SDK software to access SD card.
Note: If DATA3 is used as the card detect PIN, please make sure DATA3 is pull down, no matter internal or external, at the same time, make sure the card can pull DATA3 up, then host can detect card through DATA3.And SDHC do not support detect card through CD by host, card can be detected through DATA3 or GPIO.No matter detect card through host or gpio, make sure the pinmux configuration is correct.


Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Mini/micro USB cable
- EVKB-IMXRT1050 board
- Personal Computer
- SD card

Board settings
==============
Please insert the SDCARD into card slot(J20)
Note: 
If you want to use a SD3.0 card with SD3.0 protocol, just insert a SD3.0 card into the card slot will be ok.


Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the OpenSDA USB port on the board.
2.  Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Insert SD card to card slot 
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs successfully, the log would be seen on the terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
