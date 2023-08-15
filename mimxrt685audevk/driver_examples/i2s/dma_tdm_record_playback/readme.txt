Overview
========
The i2s_edma_tdm_record_playback example shows how to use i2s edma driver receive and playback TDM format data:

In this example, one i2s instance reocrd and one i2s instance playbacks the audio data in TDM format.

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- MIMXRT685-AUD-EVK board
- Personal Computer
- headphones with 3.5 mm stereo jack
- source of sound (line output to 3.5 mm stereo jack)
- Line in line

Board settings
==============
To make example work, please make sure board settings are applied:
  JP43 1-2, R488 1-2
  R479 1-2, R429 1-2
  R480 1-2, R481 1-2
  R510 1-2, R474 1-2
  R482 1-2, R478 1-2
  JP8 1-2, R491 1-2
  R394 1-2, R395 1-2

Prepare the Demo
================
1.  Connect headphones to J4 or J50 or J51 or J52.
2.  Connect source of sound(from PC or Smart phone) to Audio Line-In connector (J56 or J55, J57).
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
When the demo runs successfully, you can hear,

Line in J55 -> Line out J4

Line in J56 -> Line out J50

Line in J57 -> Line out J51

The log below shows example output of the I2S driver demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
I2S TDM record playback example started!


Codec Init Done.

Starting TDM record playback
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

