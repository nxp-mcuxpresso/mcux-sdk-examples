Overview
========
The mmccard FatFs FreeRTOS project is a demonstration program that uses the SDK software. It reads/writes
/erases the mmc card continuously. The purpose of this example is to show how to use MMCCARD driver
with FatFs and freeRTOS in SDK software to access MMC card.
Note: The output log of the case is not constant, since difference of card access speed and platform will affect the behavior of card access task.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

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
MMCCARD fatfs freertos example.


Make file system......The time may be long if the card capacity is big.

Create directory......

TASK1: write file successed.
TASK1: finished.
TASK2: write file successed.
TASK2: finished.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note:This log is not constant, it depends on the SOC environment/software configuration, such as the SOC core clock, the task priority.
