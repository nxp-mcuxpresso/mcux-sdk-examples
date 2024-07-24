Overview
========
This document explains the freertos_tickless example. It shows how the CPU enters the sleep mode and then
it is woken up either by expired time delay using low power timer module or by external interrupt caused by a
user defined button.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini USB cable
- FRDM-K32L2A4S board
- Personal Computer

Board settings
==============
No special is needed.

Prepare the Demo
================
1. Connect a mini USB cable between the PC host and the OpenSDA USB port on the board.
2. Open a serial terminal on PC for JLink serial device with these settings:
   - 115200 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running
   the demo.

Running the demo
================
The following message shows in the terminal if the example runs successfully.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Press any key to start the example
Tickless Demo example
Press or turn on SWx to wake up the CPU

Tick count :
0
5000
10000
CPU woken up by external interrupt
15000
20000
25000
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
