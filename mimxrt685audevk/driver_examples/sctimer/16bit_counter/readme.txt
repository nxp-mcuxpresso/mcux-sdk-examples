Overview
========
The SCTimer 16-bit counter project is a demonstration program of the SDK SCTimer driver operation when using the SCTimer counter
as two 16-bit counters.
The example toggles an output per counter when a match occurs.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Micro USB cable
- MIMXRT685-AUD-EVK board
- Personal Computer

Board settings
==============
No special settings are required.

1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J5) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows example output of the SCTimer 16-bit counter demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SCTimer example to use it in 16-bit mode

The example shows both 16-bit counters running and toggling an output periodically
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
- Probe oscilloscope at Q4-5(5HZ) and Q3-2(2.5HZ) to see output signal.
