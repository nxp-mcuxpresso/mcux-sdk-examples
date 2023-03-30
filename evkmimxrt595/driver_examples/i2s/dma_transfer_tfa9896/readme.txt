Overview
========

The I2S example project uses one I2S interface to continuously playback the sine wave to output.

Toolchain supported
===================
- MCUXpresso  11.7.0
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT595 boards
- Personal Computer
- Two 8 ohm speakers connected to J11 and J46 Separately.

Board settings
==============
To make example work, connections needed to be as follows:
JP7-3 <---> JP8-2
JP27, JP28, JP29 2-3 connected.

2. Provide power to amplifier VDD_AMP:
If JP11 is connected as 1-2, make sure J39 USB port is connected.
If JP11 is connected as 2-3, make sure J24 is supplied with external power supply.

Prepare the Demo
================
1.  Connect two 8 ohm speakers to J11 and J46 Separately.
2.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port on the board
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
Speakers will play a sine wave sound.
The log below shows example output of the I2S driver demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Configure I2S
Setup looping playback of sine wave
Initialize left TFA9896
Initialize right TFA9896
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
