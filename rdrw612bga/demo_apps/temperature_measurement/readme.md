Overview
========
The temperature measurement demo is used to measure the temperature, note that
this case just support RW61x platform at present.

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
The log below shows the output of the temperature measurement demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 Temperature measurement example.
 Please press any key to get the temperature.
 Current temperature is: 24.379 degrees Celsius.
 Current temperature is: 24.859 degrees Celsius.
 Current temperature is: 24.859 degrees Celsius.
 Current temperature is: 24.859 degrees Celsius.
 Current temperature is: 24.859 degrees Celsius.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
