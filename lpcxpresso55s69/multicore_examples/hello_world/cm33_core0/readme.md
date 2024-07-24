Overview
========
The Multicore Hello World demo application demonstrates how to set up projects for individual
cores on a multicore system. In this demo, the primary core prints the "Hello World from the Primary Core!"
string to the terminal and then releases the secondary core from the reset. The secondary
core toggles an on-board LED indicating that the secondary core is running.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso55s69 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
Note: MCUXpresso IDE project default debug console is semihost
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (P6) on the board
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
The log below shows the output of the hello world multicore demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Copy Secondary core image to address: 0x20033000, size: 3280
Starting Secondary core.

Hello World from the Primary Core!

Starting Secondary core.
The secondary core application has been started.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Note:
The "Copy Secondary core image to address..." log message is not displayed on the terminal window when MCUXpresso IDE is used.
In case of MCUXpresso IDE the secondary core image is copied to the target memory during startup automatically.
