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
- Two 8 ohm speakers connected to J11 and J20 Separately.


Board settings
==============
To make example work, connections needed to be as follows:

1. Connect P0_9_I2S_DATA_RX to I2S_DAI_AMP:
  JP8-2        <-->        JP8-3

2. Provide power to amplifier VDD_AMP:
If JP11 is connected as 1-2, make sure J6 USB port is connected.
If JP11 is connected as 2-3, make sure J24 is supplied with external power supply.


Prepare the Demo
================
1.  Connect two 8 ohm speakers to J11 and J20 Separately.
2.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J5) on the board
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
5.  Download the program to the target board.
6.  Launch the debugger in your IDE to begin running the demo.
Running the demo
================
Speakers will play a sine wave sound.
The log below shows example output of the I2S driver demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Configure TFA9XXX amplifier
...
    TFA driver log (disable if needed.)
...
Configure I2S
Setup looping playback of sine wave
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

