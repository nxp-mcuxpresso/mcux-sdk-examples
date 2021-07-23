Overview
========

The dac_interrupt example shows how to use DAC with interrupts and produce an arbitrary, user-defined waveform of
selectable frequency.The output can be observed with an oscilloscope. 

When the DAC's double-buffer feature is enabled, any write to the CR register will only load the pre-buffer, which
shares its register address with the CR register. The CR itself will be loaded from the pre-buffer whenever the 
counter reaches zero and the interrupt would occur. At the same time the counter is reloaded with the COUNTVAL
register value. In the DAC ISR, user-defined waveform array would be transfered to pre-buffer in order.



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
The log below shows the output of the DAC interrupt example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
DAC interrupt Example.
Please probe the signal using an oscilloscope.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

- Probe the signal of PIO0_17 using an oscilloscope.
