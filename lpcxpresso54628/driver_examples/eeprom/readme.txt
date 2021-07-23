Overview
========
The EEPROM Example project is a demonstration program that uses the KSDK software to program eeprom memory
and verify the program.


Toolchain supported
===================
- MCUXpresso  11.4.0
- GCC ARM Embedded  10.2.1

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
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows example output of the eeprom example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
EEPROM example begins...
Page 0 program finished!
Page 1 program finished!
Page 2 program finished!
Page 3 program finished!
Page 4 program finished!
Page 5 program finished!
Page 6 program finished!
Page 7 program finished!
Page 8 program finished!
Page 9 program finished!
...
Page 126 program finished!
All data is correct! EEPROM example succeed!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
