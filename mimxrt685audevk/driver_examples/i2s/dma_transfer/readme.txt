Overview
========

The I2S example project uses one I2S interface to continuously playback the sine wave to output.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Micro USB cable
- MIMXRT685-AUD-EVK board
- Personal Computer
- headphones with 3.5 mm stereo jack


Board settings
==============


Prepare the Demo
================
1.  Connect headphones to J4.
2.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J5) on the board
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
Headphones will play a sine wave sound.
The log below shows example output of the I2S driver demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Configure codec
Configure I2S
Setup looping playback of sine wave
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

