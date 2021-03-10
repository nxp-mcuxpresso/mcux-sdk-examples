Overview
========
The Simple Match Interrupt project is to demonstrate usage of the SDK CTimer driver with interrupt callback functions
In this example the upon match and IO pin connected to the LED is toggled and the timer is reset, so it would generate a square wave.
With an interrupt callback the match value is changed frequently in such a way that the frequency of the output square wave is increased gradually.

Toolchain supported
===================
- MCUXpresso  11.3.0
- GCC ARM Embedded  9.3.1

Hardware requirements
=====================
- Mini/micro USB cable
- LPCLPCXpresso54114 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J7) on the board
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows example output of the CTimer simple match demo using interrupts in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CTimer match example to toggle the output.
This example uses interrupt to change the match period.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
