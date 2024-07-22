Overview
========

The INTM project is a simple demonstration of the SDK INTM driver.
The role of INTM is to monitor the interrupt response. The main monitoring is whether the interrupt has timed out from 
the request to the response. Up to 4 interrupts can be monitored simultaneously.
This example is to detect the key interrupt, verify the function of INTM by delaying the time from the trigger to 
the response of the key interrupt, and confirm the interrupt timeout by the flag bit one.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXN236 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1. Connect the type-c and mini USB cable between the PC host and the USB ports on the board.
2. Open a serial terminal on PC for the serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running
   the demo.

Running the demo
================
The following lines are printed to the serial terminal when the demo program is executed.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
INTM Example Start:

Press SW2 to trigger interrupt. 
INTM timeout. 

INTM Example End.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

