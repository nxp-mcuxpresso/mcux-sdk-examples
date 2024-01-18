Overview
========
The SCTimer 16-bit counter project is a demonstration program of the SDK SCTimer driver operation when using the SCTimer counter
as two 16-bit counters.
The example toggles an output per counter when a match occurs.

To use any 16-bit counter, this project disables the Unify 32-bit Counter by hardware limit, no matter the Low 16-bit one or the
High 16-bit one. Both the Low 16-bit counters or the High 16-bit counters  enable bidirectional mode to extend the 16-bit counting
range. When the counter is in bidirectional mode, the effect of setting and clearing the output depends on whether the counter is
counting up or down.
The 16-bit low counter is scheduled for a match event every 0.1 seconds, and the 16-bit High counter is scheduled for a match event
every 0.2 seconds. When a 16-bit counter event occurs, the output is toggled and the counter is reset.

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT685 board
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
- Probe oscilloscope at Q4-5(5Hz) and Q3-2(2.5Hz) to see output signal.
