Overview
========
This example shows how to capture touch data using CAPT poll-now mode.
Comparing with the capt_basic example, this example uses ACOMP
instead of GPIO in measurement stage.
In this mode, application trigger the polling round, after polling round, the
CAPT stops and notifies application by interrupt or register status.
In this example, application poll the enabled pins one by one, after all pins
are sampled, the result are shown in the debug terminal. Then application
starts to sample all the enabled pins again.

When the example runs, the sampled data is shown in the terminal. When electrode
plate is touched, the sampled data changes too.

Toolchain supported
===================
- GCC ARM Embedded  9.3.1
- MCUXpresso  11.3.0

Hardware requirements
=====================
- Micro USB cable
- LPC845 Breakout board
- Personal Computer

Board settings
==============
Make sure R23 is on the LPC845 Breakout board.

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
