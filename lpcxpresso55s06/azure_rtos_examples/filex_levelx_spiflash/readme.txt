Overview
========
The example shows how to use FileX and LevelX with SPI flash.

This example will erase the SPI flash and format it in FAT format.
Then, do some file operation test - create, write, read, close and so on.


Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Micro USB cable
- Target Board
- Personal Computer
- One USB TTL Serial Cable
- One Flash 3 Click (a mikroBUS add-on board)

Board settings
==============
Install the Flash 3 Click board on the mikroBUS socket.

To enable the high speed SPI on the mikroBUS socket, set on-board jumpers:
* Connect JP25 pin 1 and pin 2
* Connect JP26 pin 1 and pin 2
* Connect JP9  pin 1 and pin 2
* Connect a USB TTL cable to the J3 serial port. (J3-1: RXD, J3-2: TXD, J3-3: GND)

Prepare the Demo
================
1.  Connect a USB Micro cable between the host PC and the Debug Link USB port (P6) on the target board.
2.  Connect a USB TTL cable between the host PC and the J3 serial port.
3.  Open a serial terminal on PC with the following settings (SELECT the USB TTL cable port):
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

Start FileX LevelX SPI Flash example
Erase Flash: offset = 0x0, size = 512 KB
................................................................................................................................
Fromat: disk_size = 480 KB

Creat TEST.TXT
Open TEST.TXT
Write TEST.TXT
Read TEST.TXT
Close TEST.TXT

Continue the test (y/n):

