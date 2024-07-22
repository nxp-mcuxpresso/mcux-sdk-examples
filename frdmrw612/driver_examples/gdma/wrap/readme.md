Overview
========
This project shows how to use GDMA functional APIs to do the wrap transfer.

Both the source address and destination address can be wrapped.

In this example, there are 2 buffer, s_wrappedData is 16-byte buffer,
s_unwrappedData is 64-byte buffer.

In case APP_GdmaSrcWrap, s_wrappedData is filled with 0, 1, 2, ..., 15,
s_unwrappedData is empty. GDMA copies s_wrappedData to s_unwrappedData, and the
source address wrap is set to 16. When transfer finished, the s_unwrappedData is:
0, 1, 2, ..., 15, 0, 1, 2, ..., 15, 0, 1, 2, ..., 15, 0, 1, 2, ..., 15.

In case APP_GdmaDestWrap, s_unwrappedData is filled with 0, 1, 2, ..., 63,
s_wrappedData is empty. GDMA copies s_unwrappedData to s_wrappedData, and the
destination address wrap is set to 16. When transfer finished, the s_wrappedData is:
48, 49, 50, ..., 63.

The project checks the data and shows the result in terminal.

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
