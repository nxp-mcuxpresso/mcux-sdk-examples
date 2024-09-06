Overview
========
This example works as a USB generic HID device. A PC can communicate with it.
The example has one EP IN and one EP OUT. The test script will send a message
to the device, and the device will send the received message back.


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
- One target board
- Personal Computer(PC)

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - 1 stop bit
    - No flow control
3.  Compile the demo with the configuration, "flexspi_nor_debug".
4.  Write the program to the flash of the target board.
5.  Press the reset button on your board to start the demo.
6.  Connect a USB cable between the PC and the USB device port of the board.

Running the demo
================
Connect the device to a PC through a USB cable. On the PC, run the test script, test_usbx_device_hid_generic.py

The serial of the device will output:

USBX device HID generic example
HID device activate

REV:
0x4d(M) 0x65(e) 0x73(s) 0x73(s) 0x61(a) 0x67(g) 0x65(e) 0x20( ) 
0x66(f) 0x72(r) 0x6f(o) 0x6d(m) 0x20( ) 0x48(H) 0x4f(O) 0x53(S) 
0x54(T) 

