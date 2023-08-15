Overview
========
The SDIO project is a demonstration program that uses the SDK software. It reads/writes the SDIO card reigister. The purpose of this example is to show how to use SDio driver and this is a very simple example.
Note: If the sdio card need WL_REG_ON, please connect WL_REG_ON to the sdio card VDD pin for this example.

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT595 board
- Personal Computer
- SDIO card

Board settings
==============
Make sure resistors R691~R697 are populated and resistors R611~R620,R660, R661 are removed.

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

Note:
The Rev.B. EVK board can not perform Card power reset due to board limitation.
Please insert the SDIO card into card slot(J32) after the debug console print "Please insert a card into board."

Running the demo
================
When the demo runs successfully, the log would be seen on the terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

SDIO card simple example.

Please insert a card into board.

Card inserted.

Read function CIS, in direct way

CIS read successfully
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Please note that below log maybe printed according to the card capability.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Read function CIS, in extended way, non-block mode, non-word aligned size

Read function CIS, in extended way, block mode, non-word aligned size

The read content is consistent.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
