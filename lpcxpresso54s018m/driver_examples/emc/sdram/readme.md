Overview
========

The emc_sdram example shows how to access the SDRAM.

In this example, user shall initialize the EMC (external memory controller), initialize the
EMC dynamic memory controller before access the external SDRAM.

If the sdram example fails, please make sure to check the following points:
1. Please take refer to the board.readme to check the jumper settings on your board.
2. Please take refer to the EMC chapter in API RM to do the delay calibration to found the best delay for your board, then update the delay to the EMC clock delay control registers in the system configure module registers.

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- LPCLPCXpresso54S018M board
- Personal Computer
- Jumper wire

Board settings
==============
For LPCXpresso54S018M V2.0:JP14 and JP15 2-3 on.

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
This demo is an external flash plain load demo, ROM will copy image in external flash to SRAMX to run:
1. Build the demo to generate a bin file.
   Note: If need to generate bin file using MCUXpresso IDE, below steps need to be followed:
         Set in example Properties->C/C++ Build->Settings->Build steps->Post-build steps->Edit
         enbable arm-none-eabi-objcopy -v -O binary "&{BuildArtifactFileName}" "&{BuildArtifactFileBaseName}.bin" 
         
         This plainload example linked the vector table to 0x00000000, but program to external flash 0x10000000.

2. Program the bin file to external on board flash via SEGGER J-FLASH Lite(V6.22 or higher):

   a. Open SEGGER J-FLASH Lite, select device LPC54S018M.

   b. Click the 'Erase Chip' to erase the extrenal flash.(if can not success, press SW4 button and reset the board, and try to erase again)

   c. Select the bin data file, set the '.bin/Erase Start' address to 0x10000000, then click 'Program Device'
Note: Please use above way to program the binary file built by armgcc tool chain to external flash. 
      For IAR, KEIL, MCUXpresso IDE, you can use the IDE tool to program the external flash.  
1.  Launch the debugger in your IDE to begin running the demo.


 Start EMC SDRAM access example.

 SDRAM Write Start, Start Address 0xa0000000, Data Length 2097152 !

 SDRAM Write finished!

 SDRAM Read/Check Start, Start Address 0xa0000000, Data Length 2097152 !

 SDRAM Write Data and Read Data Succeed.

 SDRAM Example End.


