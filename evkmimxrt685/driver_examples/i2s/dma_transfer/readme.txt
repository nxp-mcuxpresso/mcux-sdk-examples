Overview
========

The I2S example project uses one I2S interface to continuously playback the sine wave to output.

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT685 board
- Personal Computer
- headphones with 3.5 mm stereo jack
- source of sound (line output to 3.5 mm stereo jack)

Board settings
==============
To make example work, connections needed to be as follows:
  JP7-1        <-->        JP7-2
  JP8-1        <-->        JP8-2

Prepare the Demo
================
1.  Connect headphones to Audio HP / Line-Out connector (J4).
2.  Connect source of sound(from PC or Smart phone) to Audio Line-In connector (J3).
3.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J5) on the board
4.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
5.  Download the program to the target board.
6.  Launch the debugger in your IDE to begin running the demo.
Running the demo
================
Headphones will play a sine wave sound.
The log below shows example output of the I2S driver demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Configure codec
Configure I2S
Setup looping playback of sine wave
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

