Overview
========
The FLASIAP project is a simple demonstration program of the SDK FLASIAP driver. It erases and programs 
a portion of on-chip flash memory. A message a printed on the UART terminal as various operations on 
flash memory are performed.

Toolchain supported
===================
- MCUXpresso  11.6.0
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso54628 board
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
1.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

The following lines are printed to the serial terminal when the demo program is executed.

FLASHIAP example

Writing flash sector 1

Erasing flash sector 1

Erasing page 1 in flash sector 1

End of FLASHIAP Example
