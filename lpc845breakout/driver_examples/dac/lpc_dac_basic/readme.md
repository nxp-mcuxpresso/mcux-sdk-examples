Overview
========

The dac_basic example shows how to use DAC module simply as the general DAC converter.

When the DAC's double-buffer feature is not enabled, the CR register is used as the DAC output data register directly.
The converter would always output the value of the CR register. In this example, it gets the value from terminal,
outputs the DAC output voltage through DAC output pin.

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
- LPC845 Breakout board
- Personal Computer

Board settings
==============
PIO0_17 is DAC0 output pin.

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
The log below shows the output of the DAC basic driver demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
DAC basic Example.

Please input a value (0 - 1023) to output with DAC:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Probe the signal of PIO0_17 using an oscilloscope.
