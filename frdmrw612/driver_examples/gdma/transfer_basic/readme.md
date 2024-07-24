Overview
========
This project shows how to use GDMA transactional APIs to do the basic memory to memory
data transfer. In this example, GDMA copies data from one buffer to another buffer,
when copy finished, the two buffer content are compared to make sure the transfer
succeed. The compare result is shown in the terminal.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- FRDM-RW612 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J10) on the board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the example.

Running the demo
================
The example run result will be shown in the terminal.
