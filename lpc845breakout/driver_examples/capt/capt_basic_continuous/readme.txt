Overview
========
This example shows how to capture touch data using CAPT continuous mode.
CAP polls and samples the touch pad data round by round, in each round, all the
enabled pins are sampled one by one. To monitor one pad sample finished,
both the yes_touch and no_touch interrupt is enabled. The poll done interrupt
is enabled to monitor the poll round finished. The delay period between two
polling round is configurable, in this example, the delay period is set to
about 0.5s.
When the example runs, the sampled data is shown in the terminal, and update
every about 0.5s. When electrode plate is touched, the sampled data changes too.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.4.0

Hardware requirements
=====================
- Micro USB cable
- LPC845 Breakout board
- Personal Computer

Board settings
==============
Remove R23 on the LPC845 Breakout board.

Prepare the demo
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port on the board.
2.  Open a serial terminal with the following settings:
    - 9600 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The CAPT sample data is shown in the terminal, touch the electrode plate to see
sample data change.
