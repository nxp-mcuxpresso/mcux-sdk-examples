Overview
========
This driver example demonstrates how to setup PRINCE driver for the on-the-fly encryption/decryption of data stored in the internal flash memory. It shows how to enable encryption/decryption 
for specified flash memory address range, how to generate new IV code and how to load the IV code into the PRINCE bus encryption engine.
The example also shows how to correctly perform PRINCE region erase and write operations with dummy data.

Toolchain supported
===================
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso55S36 board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J1) on the board
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
PRINCE ROM Example.
PRINCE Peripheral Driver Example

Calling API_Init
API_Init Successfully
///////////////////////////////////////////  CAUTION!!!  ///////////////////////////////////////////////
Once the user decides to enable ROM PRINCE feature, ROM does not accept to de-enable PRINCE for no encrypted boot,
i.e., if PRINCE is used via ROM feature, and if you try to boot without PRINCE, that may cause boot fail!!
//////////////////////////////////////////////////////////////////////////////////////////////////////////
Press any key to continue
Configure PRINCE enc/dec: start 0x10000 size 8192
Configure PRINCE  Successfully
Flash erased success
Flash encrypted write success
Flush memory success
Decrypted data read successfully
Encrypted data read successfully
Reconfigure PRINCE  Successfully
Decrypted data read successfully

Example end.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Note: Avoid using  NS: 0x20002000-0x20004000 & S:0x30002000-0x30004000 address ranges, since it's PKC RAM.
Note: Configuring PRINCE for on the fly encryption/decryption is expected to be called once in the device lifetime, 
typically during the initial device provisioning, since it is programming the CMPA pages in PFR flash.
