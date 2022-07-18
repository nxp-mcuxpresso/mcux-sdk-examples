Overview
========
This example shows how to use CAPT to detect key touch event.
When the key is touched, LED on the board is on accordingly.
The example first runs the calibration, in this period, all LEDs
are on, please don't touch any key at this time. After all LEDs off
the example starts touch detection, touch any key and the LED turns on.
This example only supports touch one key at the same time.
CAPT sample data are saved in a window for later process.
During calibration period, the variance of each channel is used to judge
whether the channel is stable. Calibration finished only when all channels
are stable. The stable value after calibration is saved as baseline, to
be used in the detection stage.
To remove touch glitch, a software filter is used in this example.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

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
When touch the electrode plate, the corresponding LED would turn on.
