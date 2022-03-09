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
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Micro USB cable
- evkbimxrt1050 board
- Personal Computer
- SDIO card(such as wifi card, we use ATHEROS AR6233X to test)

Board settings
==============
Insert the sdio card into card slot.
Note that sdcard connector is for micro sd,but sdio connector is normal one, so you need a connector converter to connect.

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

SDIO freertos example.

Card inserted.

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

SDIO Pending interrupt 0

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
