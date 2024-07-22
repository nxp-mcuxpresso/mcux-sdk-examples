Overview
========
The TMPSNS project is a simple demonstration program of the SDK TMPSNS driver.The
temperature sensor (TMPSNS) module features alarm functions that can raise independent
interrupt signals if the temperature is above the high-temperature thresholds threshold,
the system can then use this module to monitor the on-die temperature and take appropriate actions.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

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
