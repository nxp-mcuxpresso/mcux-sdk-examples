Overview
========
The SDCARD FATFS project is a demonstration program that uses the SDK software. It mounts a file 
system based on a SD card then does "creat directory/read directory/create file/write file/read file"
operation. The file sdhc_config.h has default SDHC configuration which can be adjusted to let card
driver has different performance. The purpose of this example is to show how to use SDCARD driver 
based FATFS disk in SDK software.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1180-EVK board
- Personal Computer
- SD card

Board settings
==============
1.Please make sure R257/R140 is weld for GPIO card detect.
2.Please make sure J76 1-2 and J57 2-3 are installed for sdcard socket.
Note:
As the EVK board limitaion, there is eletronic mux added for switch between M.2 and sdcard which will affect the sdcard SDR104 timing.
So the maximum sd card timing frequency been decreased to 100MHZ to improve the stability.
User can remove the limitation by change the macro BOARD_SDMMC_SD_HOST_SUPPORT_SDR104_FREQ in sdmmc_config.h from (200000000U / 2U) to (200000000U).


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

FATFS example to demonstrate how to use FATFS with SD card.

Please insert a card into board.
Detected SD card inserted.

Makes file system......This time may be long if the card capacity is big.

Creates directory......

Creates a file in that directory......

Lists the file in that directory......
General file : F_1.DAT.
Directory file : DIR_2.

Writes/reads file until encounters error......

Writes to above created file.
Reads from above created file.
Compares the read/write content......
The read/write content is consistent.
Input 'q' to quit read/write.
Input other char to read/write file again.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
