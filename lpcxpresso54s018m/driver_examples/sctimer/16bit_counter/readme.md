Overview
========
The SCTimer 16-bit counter project is a demonstration program of the SDK SCTimer driver operation when using the SCTimer counter
as two 16-bit counters.
The example toggles an output per counter when a match occurs.

To use any 16-bit counter, this project disables the Unify 32-bit Counter by hardware limit, no matter the Low 16-bit one or the
High 16-bit one. Both the Low 16-bit counters or the High 16-bit counters  enable bidirectional mode to extend the 16-bit counting
range. When the counter is in bidirectional mode, the effect of setting and clearing the output depends on whether the counter is
counting up or down.
The 16-bit low counter is scheduled for a match event every 0.1 seconds, and the 16-bit High counter is scheduled for a match event
every 0.2 seconds. When a 16-bit counter event occurs, the output is toggled and the counter is reset.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

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
4.  Launch the debugger in your IDE to begin running the demo.

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
The log below shows example output of the SCTimer 16-bit counter demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SCTimer example to use it in 16-bit mode

The example shows both 16-bit counters running and toggling an output periodically
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Probe oscilloscope at J13-5(2.5Hz) and J13-16(5Hz) to see output signal.
