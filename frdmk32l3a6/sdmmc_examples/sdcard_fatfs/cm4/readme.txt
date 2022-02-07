Overview
========
The SDCARD FATFS project is a demonstration program that uses the SDK software. It mounts a file 
system based on a SD card then does "creat directory/read directory/create file/write file/read file"
operation. The file sdhc_config.h has default SDHC configuration which can be adjusted to let card
driver has different performance. The purpose of this example is to show how to use SDCARD driver 
based FATFS disk in SDK software.

Toolchain supported
===================
- MCUXpresso  11.4.0
- GCC ARM Embedded  10.2.1

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

FATFS example to demonstrate how to use FATFS with SD card.

Please insert a card into board.
Detected SD card inserted.

Makes file system......This time may be long if the card capacity is big.

Creates directory......

Creates a file in that directory......

Lists the file in that directory......
Directory file : ..
Directory file : ...
General file : F_1.DAT.

Writes/reads file until encounters error......

Writes to above created file.
Reads from above created file.
Compares the read/write content......
The read/write content is consistent.
Input 'q' to quit read/write.
Input other char to read/write file again.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
