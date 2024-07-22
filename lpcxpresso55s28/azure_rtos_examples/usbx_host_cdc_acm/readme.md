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
This example can work with the USB high speed port (P9)or the USB full speed port (P10).
High speed: Install jumper in position 1-2 pins of J6 and open J7 jumper.
Full speed: Install jumper in position 2-3 pins of J6 and short J7 pins.

Prepare the Demo
================
1.  High speed: Connect the USB A Female to Micro B Male cable between a USB CDC ACM device and
                the on-board USB high speed port (P9).
    Full speed: Connect the USB A Female to Micro B Male cable between a USB CDC ACM device and
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
When the demo is running, the serial port will output:

Start the USBX HOST CDC ACM example...

Then, if connecting a device running the usbx_device_cdc_acm example to
the USB device port of the board. The serial port will output:

USB device: vid=0x8484, pid=0x0
SEND: A
RECV: Aabcdef

