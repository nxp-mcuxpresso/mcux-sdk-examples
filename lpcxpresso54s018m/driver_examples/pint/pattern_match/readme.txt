Overview
========
This example shows how to use SDK drivers to use the Pin interrupt & pattern match peripheral.

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso54S018M board
- Personal Computer

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

The following lines are printed to the serial terminal when the demo program is executed.

PINT Pattern Match example

PINT Pattern match events are configured

Press corresponding switches to generate events

2. This example configures "Pin Interrupt 0" to be invoked when SW2 switch is pressed by the user. 
   Bit slice 0 is configured as an endpoint in sticky falling edge mode. The interrupt callback prints 
   "PINT Pin Interrupt 0 event detected. PatternMatch status =        1". 
   
   "Pin Interrupt 2" is configured to be invoked when rising edge on SW2, SW3 is detected. The 
   interrupt callback prints "PINT Pin Interrupt 2 event detected. PatternMatch status =     100". Bit slices
   1 configured to detect sticky rising edge. Bit slice 2 is configured as an endpoint.

