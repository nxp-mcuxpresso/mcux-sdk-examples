Overview
========
This driver example demonstrates how to setup PRINCE driver for the on-the-fly encryption/decryption of data stored in the internal flash memory. It shows how to enable encryption/decryption 
for specified flash memory address range, how to generate new IV code and how to load the IV code into the PRINCE bus encryption engine.
The example also shows how to correctly perform PRINCE region erase and write operations with dummy data.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso55S06 board
- Personal Computer

Board settings
==============

The keystore in PFR (Protected Flash Region) has to be created before this application is executed.
Refer to the device user manual (UM11295) for more information about how to create the keystore.
The general steps are:
1. Install the JS3 jumper to allow the ISP mode after the reset.
2. Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J1) on the board.
3. Locate the middleware\mcu-boot\bin\Tools\blhost\win\blhost.exe file. If this PC tool is not available in your package,
   visit the https://mcuxpresso.nxp.com and build the MCUXpresso SDK package with the mcu-boot optional component again.    
4. Open the Command Prompt, move to the folder with the blhost.exe and apply the following commands 
   (See Appendix A in Getting started guide for description how to determine comX number):
   blhost -p comX -- key-provisioning enroll 
   blhost -p comX -- key-provisioning set_key 7 16 
   blhost -p comX -- key-provisioning set_key 8 16 
   blhost -p comX -- key-provisioning set_key 9 16 
   blhost -p comX -- key-provisioning write_key_nonvolatile 0 
5. Remove the JS3 jumper and reset the board.

Note, the keystore is created once and if stored in NVM it is not necessary to apply above mentioned step before every run of the PRINCE example.

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
PRINCE Example.
PRINCE engine should be initialized by the bootloader at this point if the activation code is valid and the PUF Enroll had been performed. Here is the actual PRINCE configuration:
PRINCE region 0 base address: 0x0
PRINCE region 0 SR enable mask: 0x0
PRINCE region 1 base address: 0x0
PRINCE region 1 SR enable mask: 0x0
PRINCE region 2 base address: 0x0
PRINCE region 2 SR enable mask: 0x0

PRINCE was successfully configured for on-the-fly encryption/decryption from 0x30000 to 0x32000.

New PRINCE configuration:
PRINCE region 0 base address: 0x0
PRINCE region 0 SR enable mask: 0x0
PRINCE region 1 base address: 0x0
PRINCE region 1 SR enable mask: 0x1000000
PRINCE region 2 base address: 0x0
PRINCE region 2 SR enable mask: 0x0

Example end.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
