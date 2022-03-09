Overview
========

The I2S example project uses one I2S interface to continuously record input sound to a buffer
and another I2S interface to playback the buffer to output - digital loopback.

Toolchain supported
===================
- MCUXpresso  11.5.0
- GCC ARM Embedded  10.2.1

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso54S018M board
- Personal Computer
- headphones with 3.5 mm stereo jack
- source of sound (line output to 3.5 mm stereo jack)


Board settings
==============
No special settings are required on LPCXpresso54S018M board.

Prepare the Demo
================
1.  Connect headphones to Audio HP / Line-Out connector.
2.  Connect source of sound(from PC or Smart phone) to Audio Line-In connector.
3.  Connect a micro USB cable between the host PC and the LPC-Link USB port (J8) on the target board.
4.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
5.  Download the program to the target board.
6.  Launch the debugger in your IDE to begin running the demo.

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

Headphones will play what is input into Audio Line-In.
Terminal window will show:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Configure codec
Configure I2S
Setup digital loopback
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
