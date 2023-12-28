Overview
========
The lcdc_cursor example shows how to use LCD hardware cursor.

In this example, a 32x32 cursor is shown. The cursor's position is changed at
the end of every frame.

The background is:
+------------------------------------------------+
+                                                +
+                    Red                         +
+                                                +
+            +--------------------+              +
+            +                    +              +
+            +      Blank         +              +
+            +                    +              +
+            +--------------------+              +
+                                                +
+                                                +
+                                                +
+------------------------------------------------+

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
- LPCXpresso54S018M board
- Personal Computer
- RK043FN02H-CT or RK043FN66HS-CT6 LCD board

Board settings
==============
No special board settings.

Prepare the Demo
================
1. Connect a micro USB cable between the PC host and the LPC-Link USB port (J8) on the board.
2. Download the program to the target board.
3. Launch the debugger in your IDE to begin running
   the demo.

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
There is a cursor moves in the panel if the example runs successfully.
