Overview
========
The IAP FAIM project is a simple demonstration program of the SDK IAP
driver. It writes and reads the FAIM page. A message a printed on the UART terminal
as FAIM read and write operations are performed.

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso845MAX board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a micro USB cable between the host PC and the CMSIS DAP port(J4 on the board).
2.  Open a serial terminal with the following settings:
    - 9600 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.

Running the demo
================
1.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

The following lines are printed to the serial terminal when the demo program is executed.

IAP FAIM example

Write FAIM sector 1

Read FAIM sector 1

End of IAP FAIM Example 
