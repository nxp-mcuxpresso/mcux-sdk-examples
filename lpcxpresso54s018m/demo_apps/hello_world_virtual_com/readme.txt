Overview
========
Hello World Virtual Com demonstrates the use of virtual com to print the "Hello World" string to the terminal.
Different with Hello World demo (a HW UART peripheral is used to transmit data), the demo transmits data based on
a serial port simulated by the USB device stack on target board side via USB peripheral.
The purpose of this demo is to demonstrate how to use virtual com and provide a simple project for debugging and further development. 

Toolchain supported
===================
- MCUXpresso  11.5.0
- GCC ARM Embedded  10.2.1

Hardware requirements
=====================
- Two Micro USB cables
- LPCXpresso54S018M board
- Personal Computer

Board settings
==============
Please make sure the JP9 is set up to HS mode.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board for downloading
    the program and supply power for the board, connect another USB cable between host PC and USB port
	on the target board.
2.  Download the program to the target board.
3.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.
4.  Open a serial terminal(like putty) to communicate with the board via virtual com.

Running the demo
================
The log below shows the output of the hello world demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
hello world.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The debug console virtual com will echo back the received characters.
