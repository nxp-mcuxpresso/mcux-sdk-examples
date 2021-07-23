Overview
========
The freertos sdio used to demonstrate how to use SDK sdio driver with freertos.
There are three tasks in this example:
CardDetectTask
AccessCardTask
CardInterruptTask
CardDetectTask is used to monitor the card insertion event, if the card is not inserted, it will wait until card insert and then this task will perform card initialization and release a card access semaphore, then continuous wait card inserts or remove event and going to sleep.
AccessCardTask is used to access the sdio card, after card access semaphore is taken, it will perform some simple card access operation, then release the card access semaphore and then going sleep to wait for user input.
CardInterruptTask is used to monitor the sdio interrupt event, this task need take two semaphore card access semaphore(which is released in other tasks) and card interrupt semaphore(which is released in card interrupt ISR), if the two semaphores is taken, it will perform a simple read to check the pending io interrupt, then release a card access semaphore and continuous wait card interrupt.
In this example, it demonstrates to detect card insertion/access card/card interrupt handling.
Note: If the sdio card need WL_REG_ON, please connect WL_REG_ON to the sdio card VDD pin for this example.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.4.0

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

SDIO freertos example.

Card inserted.

Card init finished, ready to access.

Read function CIS, in direct way

Read function CIS, in extended way, non-block mode, non-word aligned size

Read function CIS, in extended way, block mode, non-word aligned size

The read content is consistent.

Input 'q' to quit card access task.

Input other char to access again.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If sdio card interrupt is generated, then more log will be generated like:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

SDIO interrupt is received

IO 1 interrupt detected.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If sdio card is not inserted before example run, then more log will generated like:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SDIO freertos example.

Please insert a card into board.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
