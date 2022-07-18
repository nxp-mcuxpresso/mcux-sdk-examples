Overview
========

The emc_sdram example shows how to access the SDRAM.

In this example, user shall initialize the EMC (external memory controller), initialize the
EMC dynamic memory controller before access the external SDRAM.

If the sdram example fails, please make sure to check the following points:
1. Please take refer to the board.readme to check the jumper settings on your board.
2. Please take refer to the EMC chapter in API RM to do the delay calibration to found the best delay for your board, then update the delay to the EMC clock delay control registers in the system configure module registers.

Toolchain supported
===================
- MCUXpresso  11.6.0
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Micro USB cable
- LPCLPCXpresso54628 board
- Personal Computer
- Jumper wire

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a micro USB cable between the host PC and the LPC-Link USB port (J8) on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.

Running the demo
================
1.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.


 Start EMC SDRAM access example.

 SDRAM Write Start, Start Address 0xa0000000, Data Length 2097152 !

 SDRAM Write finished!

 SDRAM Read/Check Start, Start Address 0xa0000000, Data Length 2097152 !

 SDRAM Write Data and Read Data Succeed.

 SDRAM Example End.


