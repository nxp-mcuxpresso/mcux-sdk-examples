Overview
========
Enable multicore support for Glow machine learning security  for Microcontrollers

The inference process is run on Cortex-M7 core, and  capture the image data  is run
on Cortex-M4 core. M7 core starts up first and loads M4 core image into SDRAM (0x20200000),
then M4 core starts up using the image in SDRAM.


Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- EVK-MIMXRT1170 board
- Personal computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1. Connect a USB cable between the host PC and the OpenSDA USB port on the target board. 
2. Open a serial terminal with the following settings:
   - 115200 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows the output of the demo in the terminal window (compiled with ARM GCC):
eiq multicore secure ml 9 (94%) (22:48:01)
eiq multicore secure ml 6 (89%) (22:48:01)
eiq multicore secure ml 9 (94%) (22:48:01)
eiq multicore secure ml 6 (89%) (22:48:01)
eiq multicore secure ml 9 (94%) (22:48:01)
eiq multicore secure ml 6 (89%) (22:48:01)
eiq multicore secure ml 9 (94%) (22:48:01)
eiq multicore secure ml 6 (89%) (22:48:01)
eiq multicore secure ml 9 (94%) (22:48:01)

