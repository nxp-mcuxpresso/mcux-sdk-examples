Overview
========
The example shows how to use the SD card middleware with Azure RTOS.

This example will print the information of the SD card and format it
in FAT format. Then, do some file operation test - create, write,
read, close and so on.


Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Mini/micro USB cable
- EVKB-IMXRT1050 board
- Personal Computer
- Micro SD card

Board settings
==============
Please insert a micro SD card into the card slot.

Prepare the Demo
================
1.  Insert a micro SD card into the card slot
2.  Connect a micro USB cable between the PC host and the OpenSDA USB port on the board.
3.  Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

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
