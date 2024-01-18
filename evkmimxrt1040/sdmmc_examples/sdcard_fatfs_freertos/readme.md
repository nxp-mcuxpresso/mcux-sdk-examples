Overview
========
The SDCARD FatFs FreeRTOS project is a demonstration program that uses the SDK software. It reads/writes
/erases the SD card continuously. The purpose of this example is to show how to use SDCARD driver
with FatFs and freeRTOS in SDK software to access SD card.
Note: The output log of the case is not constant, since difference of card access speed and platform will affect the behavior of card access task.

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Mini/micro USB cable
- EVKM-IMXRT1040 board
- Personal Computer
- SD card

Board settings
==============
Please insert the SDCARD into card slot(J20)
Note:
If you want to use a SD3.0 card with SD3.0 protocol, just insert a SD3.0 card into the card slot will be ok.
As the EVK board limitaion, there is eletronic mux added for switch between M.2 and sdcard which will affect the sdcard SDR104 timing.
So the maximum sd card timing frequency been decreased to 100MHZ to improve the stability.
User can remove the limitation by change the macro BOARD_SDMMC_SD_HOST_SUPPORT_SDR104_FREQ in sdmmc_config.h from (100000000U) to (200000000U).


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

SDCARD fatfs freertos example.

Card inserted.

Make file system......The time may be long if the card capacity is big.

Create directory......
TASK1: write file successed.
TASK2: write file successed.
TASK1: finsied.
TASK2: finsied.


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
