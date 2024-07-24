Overview
========

The lwip_dhcp demo application demonstrates a DHCP and ping demo on the lwIP TCP/IP and USB stack.
The application acts as a DHCP client and prints the status as it is progressing.
Once the interface is being bound to an IP address obtained from DHCP server, address information is printed.
The application will get ip adress www.nxp.com and ping the ip address.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso54s018 board
- Personal Computer

Board settings
==============
  - The Jumper settings:\n J9 1-2 high speed mode,2-3 full speed mode; J10 open high speed, shunt full speed; J11 2-3, J12 2-3, J13 2-3 , need provide external power through J1.
Prepare the Demo
================
1.  Connect a USB cable between the PC host and the OpenSDA(or USB to Serial) USB port on the target board.
2.  Open a serial terminal on PC for OpenSDA serial(or USB to Serial) device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
refer to MCUXpresso SDK USB RNDIS & LWIP User Guide
