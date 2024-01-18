Overview
========
This document explains the freertos_event example. It shows how task waits for an event (defined set
of bits in event group). This event can be set by any other process or interrupt in the system.

The example application creates three tasks. Two write tasks write_task_1 and write_task_2
continuously setting event bit 0 and bit 1.

Read_task is waiting for any event bit and printing actual state on console. Event bits are
automatically cleared after read task is entered.

Three possible states can occurre:
Both bits are set.z
Bit B0 is set.
Bit B1 is set.


SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

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
Bit B1 is set.
Bit B0 is set.
Bit B1 is set.
Bit B0 is set.
Bit B1 is set
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
