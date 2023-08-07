Overview
========
The RIT project is a simple demonstration program of the SDK RIT driver. It sets up the RIT
hardware block to trigger a periodic interrupt at 1 second period interval to toggel a specified
LED on board.

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

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
The log below shows example output of the RIT driver demo in the terminal window and see LED1 blinks at 1s period.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
RIT Example Start, You will see spcified LED blink at 1s period interval.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
