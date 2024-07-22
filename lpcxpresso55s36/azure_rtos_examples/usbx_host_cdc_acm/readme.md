Overview
========
This example works as a USB host CDC ACM. It can communicate with a USB CDC ACM device.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Two Micro USB cables
- One USB A Female to Micro B Male cable
- A USB CDC ACM device
- Target Board
- Personal Computer(PC)

Board settings
==============
Short the JP2 jumper.

Prepare the Demo
================
1.  Connect the USB A Female to Micro B Male cable between a USB CDC ACM device and
    the on-board USB full speed port (J3).
2.  Connect a USB Micro cable between the host PC and the Debug Link USB port (J1) on the target board.
3.  Connect a USB Micro cable between the host PC and the on-board USB Power (J2).
4.  Open a serial terminal on the PC with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
5.  Compile the demo.
6.  Download the program to the target board.
7.  Press the on-board RESET button to start the demo.

Running the demo
================
When the demo is running, the serial port will output:

Start the USBX HOST CDC ACM example...

Then, if connecting a device running the usbx_device_cdc_acm example to
the USB device port of the board. The serial port will output:

USB device: vid=0x8484, pid=0x0
SEND: A
RECV: Aabcdef

