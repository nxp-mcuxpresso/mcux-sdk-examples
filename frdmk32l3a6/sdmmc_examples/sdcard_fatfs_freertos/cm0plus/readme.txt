Overview
========
The SDCARD FatFs FreeRTOS project is a demonstration program that uses the SDK software. It reads/writes
/erases the SD card continuously. The purpose of this example is to show how to use SDCARD driver
with FatFs and freeRTOS in SDK software to access SD card.
Note: The output log of the case is not constant, since difference of card access speed and platform will affect the behavior of card access task.

Toolchain supported
===================
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

Create a directory in that directory......

List the file in that directory......
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
