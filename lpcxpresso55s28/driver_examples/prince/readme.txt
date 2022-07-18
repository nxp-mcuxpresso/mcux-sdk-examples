Overview
========
This driver example demonstrates how to setup PRINCE driver for the on-the-fly encryption/decryption of data stored in the internal flash memory. It shows how to enable encryption/decryption 
for specified flash memory address range, how to generate new IV code and how to load the IV code into the PRINCE bus encryption engine.
The example also shows how to correctly perform PRINCE region erase and write operations with dummy data.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso55S28 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
Note: MCUXpresso IDE project default debug console is semihost
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (P6) on the board
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows example output of the random number generator demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PRINCE Example.
PRINCE engine should be initialized by the bootloader at this point if the activation code is valid and the PUF Enroll had been performed. Here is the actual PRINCE configuration:
PRINCE region 0 base address: 0x0
PRINCE region 0 SR enable mask: 0x0
PRINCE region 1 base address: 0x0
PRINCE region 1 SR enable mask: 0x0
PRINCE region 2 base address: 0x0
PRINCE region 2 SR enable mask: 0x0

PRINCE was succesfully configured for on-the-fly encryption/decryption from 0x40000 to 0x42000.
Example end.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
