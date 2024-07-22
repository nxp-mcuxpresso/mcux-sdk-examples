Overview
========
This project shows how to use GDMA transactional APIs to do the memory to memory
data transfer with link list feature.

In this example, there are 4 buffers, s_gdmaData0 to s_gdmaData3. At the beginning,
only s_gdmaData0 is filled with data. GDMA copies s_gdmaData0 to s_gdmaData1,
then copies s_gdmaData1 to s_gdmaData2, and copies s_gdmaData2 to s_gdmaData3.
So at last the data in 4 buffers should be the same.

The project checks the data in each buffer and show the result terminal.

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
- RD-RW61X-BGA board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J7) on the board.
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
