Overview
========
The example shows how to use the SD card middleware with Azure RTOS.

This example will print the information of the SD card and format it
in FAT format. Then, do some file operation test - create, write,
read, close and so on.


Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- One Micro USB cables
- Target Board
- Personal Computer(PC)
- Micro SD card

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Insert a micro SD card into the card slot
2.  Connect a USB Micro cable between the host PC and the Debug Link USB port (P6) on the target board.
3.  Open a serial terminal on PC with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Compile the demo.
5.  Download the program to the target board.
6.  Press the on-board RESET button to start the demo.

Running the demo
================
When the example runs successfully, the console will output like:

Start FileX SD Card example
Please insert a SD card.
SD card inserted.
Card size: 15728 MB
Card block size: 512 bytes
Card block count: 30720000
Voltage: 1.8V
Timing mode: SDR104
Formatted SD Card

Creat TEST.TXT
Open TEST.TXT
Write TEST.TXT
Read TEST.TXT
Close TEST.TXT

Continue the test (y/n):

If you want to test again, plese select 'y'.
If you want to remove the SD card, please select 'n' to end the test.
