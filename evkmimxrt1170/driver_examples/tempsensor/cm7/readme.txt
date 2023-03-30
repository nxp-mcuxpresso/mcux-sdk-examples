Overview
========
The TMPSNS project is a simple demonstration program of the SDK TMPSNS driver.The
temperature sensor (TMPSNS) module features alarm functions that can raise independent
interrupt signals if the temperature is above the high-temperature thresholds threshold,
the system can then use this module to monitor the on-die temperature and take appropriate actions.

Toolchain supported
===================
- MCUXpresso  11.7.0
- GCC ARM Embedded  10.3.1

Board settings
==============

Prepare the Demo
================
1. Connect a USB cable between the PC host and the OpenSDA USB port on the board.
2. Open a serial terminal with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
When the example runs successfully, the following message is displayed in the terminal:
~~~~~~~~~~~~~~~~~~~~~~~~
TMPSNS driver example. 
The chip initial temperature is 23.4 ℃. 
The chip temperature has reached high temperature that is 30.9 ℃. 

~~~~~~~~~~~~~~~~~~~~~~~

Note 1:
To run this exmaple successfully, you should heating the chip outside, such as blowing heating.

Note 2:
To debug in qspiflash, following steps are needed:
1. Select the flash target and compile.
3. Set the SW1: 1 off 2 off 3 on 4 off, then power on the board and connect USB cable to J11.
4. Start debugging in IDE.
   - Keil: Click "Download (F8)" to program the image to qspiflash first then clicking "Start/Stop Debug Session (Ctrl+F5)" to start debugging.
