Overview
========
The ELEMU Example project is a demonstration program that uses the KSDK
software to show basic communication with Security Sub-System (SSS) 
and usage of SSS services to encrypt data by AES CBC with direct use
of ELEMU driver.


SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXW71 Board
- Personal Computer

Board settings
==============
TZM_EN fuse needs to be burned before execution.

Prepare the Demo
================
1. Connect the micro and mini USB cable between the PC host and the USB ports on the board.
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
ELEMU Peripheral Driver Example
SUCCESS: expected result of AES CBC encrypted data is equal to value returned by Security Sub-System!!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


