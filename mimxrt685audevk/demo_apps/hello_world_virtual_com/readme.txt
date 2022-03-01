Overview
========
Hello World Virtual Com demonstrates the use of virtual com to print the "Hello World" string to the terminal.
Different with Hello World demo (a HW UART peripheral is used to transmit data), the demo transmits data based on
a serial port simulated by the USB device stack on target board side via USB peripheral.
The purpose of this demo is to demonstrate how to use virtual com and provide a simple project for debugging and further development. 

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Two Micro USB cables
- MIMXRT685-AUD-EVK board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port(J5) on the target board for downloading
    the program and supply power for the board, connect another USB cable between host PC and USB port(J7)
	on the target board.
2.  Download the program to the target board.
3.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.
4.  A COM port is enumerated in the Device Manager. If it prompts for CDC driver installation, please reference the virtual_com_readme.pdf which can be found at below directory:
    In Release package: <install_dir>\devices\<soc>\utilities\virtual_com_readme.pdf .to install the CDC driver.
5.  Open the COM port in a terminal tool, such as TeraTerm, SSCOM, Putty, Putty is recommended while virtual com
is used.
6.  Then user can communicate with target boards through virtual com.

Running the demo
================
The log below shows the output of the hello world demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
hello world.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
The debug console virtual com will echo back the received characters.
