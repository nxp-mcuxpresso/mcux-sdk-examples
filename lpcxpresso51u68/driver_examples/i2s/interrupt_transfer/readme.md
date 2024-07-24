Overview
========

The I2S example project uses one I2S interface to continuously playback the sine wave to output.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso51U68 board
- NXP Mic/Audio/Oled (MAO) shield
- Personal Computer
- headphones with 3.5 mm stereo jack
- source of sound (line output to 3.5 mm stereo jack)


Board settings
==============
No special settings are required on LPCXpresso51U68 board.
MAO shield should be mounted to LPCXpresso51U68 board.
MAO shield - JP3 1-2 have to be connected.

Prepare the Demo
================
1.  Connect headphones to Audio HP / Line-Out connector of MAO shield.
2.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J6) on the board
3.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
Headphones will play a sine wave sound.
This demo outputs 450HZ sine wave audio signal.
The log below shows example output of the I2S driver demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Configure codec
Configure I2S
Setup looping playback of sine wave
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

