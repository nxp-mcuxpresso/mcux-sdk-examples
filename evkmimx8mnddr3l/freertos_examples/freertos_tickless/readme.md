Overview
========
This document explains the freertos_tickless example. It shows how the CPU enters the sleep mode and then
it is woken up either by expired time delay using low power timer module or by external interrupt caused by a
user defined button.


SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- MIMX8MN6-EVK  board
- J-Link Debug Probe
- 12V power supply
- Personal Computer

Board settings
==============
No special settings are required.



Prepare the Demo
================
1.  Connect 12V power supply and J-Link Debug Probe to the board, switch SW101 to power on the board
2.  Connect a USB cable between the host PC and the J901 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs successfully, the following message is displayed in the terminal:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Press any key to start the example
Tickless Demo example

Tick count :
1
5001
10001
15001
20001
25001
30001
35001
40001
45001
50001
...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
