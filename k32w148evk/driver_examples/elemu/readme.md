Overview
========
The ELEMU Example project is a demonstration program that uses the KSDK
software to show basic communication with Security Sub-System (SSS) 
and usage of SSS services to encrypt data by AES CBC with direct use
of ELEMU driver.


SDK version
===========
- Version: 2.15.000

Toolchain supported
===================
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- K32W148-EVK Board
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
ELEMU Peripheral Driver Example:
SUCCESS: expected result is equal to value provided by Security Sub-System!!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


