Overview
========
This example illustrates USBX Host Mass Storage.


Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Two Micro USB cables
- One USB A Female to Micro B Male cable
- A U-disk device
- Target Board
- Personal Computer(PC)

Board settings
==============
This example can work with the USB high speed port (P9) or the USB full speed port (P10).
High speed: Install jumper in position 1-2 pins of J6 and open J7 jumper.
Full speed: Install jumper in position 2-3 pins of J6 and short J7 pins.

Prepare the Demo
================
1.  High speed: Connect the USB A Female to Micro B Male cable between the U-disk device and
                the on-board USB high speed port (P9).
    Full speed: Connect the USB A Female to Micro B Male cable between the U-disk device and
                the on-board USB full speed port (P10).
2.  Connect a USB Micro cable between the host PC and the Debug Link USB port (P6) on the target board.
3.  Connect a USB Micro cable between the PC and the on-board USB power port (P5).
4.  Open a serial terminal on the PC with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
5.  Compile the demo:
    - High speed: Set USB_HOST_CONFIG_IP3516HS to 1 in board_setup.c
    - Full speed: Set USB_HOST_CONFIG_IP3516HS to 0 in board_setup.c
6.  Download the program to the target board.
7.  Press the on-board RESET button to start the demo.

Running the demo
================
After writing the program to the flash of the target board,
press the reset button on your board to start the demo.
The serial port will output:

USBX host mass storage example

Then, connect a U-disk to the USB device port of the board.
The example will display the vendor ID and the product ID of
the attached USB device, then scan the disk and dump the file
name to the serial port.

For example:

USB device: vid=0x2ce3, pid=0x6487
Find Dir: DIR_1
Find Dir: DIR_2
Find File: 1234.text
Find File: test.text

