Overview
========
The sai_interrupt_record_playback example shows how to use sai driver with record and playback features:

In this example, one sai instance record the audio data from input and playbacks the audio data.

Toolchain supported
===================
- MCUXpresso  11.6.0
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVK board
- Personal Computer
- Headphone(OMTP standard)

Board settings
==============
To make the examples works, please remove below resistors if on board wifi chip is not DNP:
R228,R229,R232,R234

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4. Insert the headphones into the headphone jack on MIMXRT1170-EVK board (J33).
5. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================

Note: This demo uses both headphone mic and board main mic(P1) as input source. The headphone mic provides left
channel data, and main mic (P1) provides right channel data. If users found there is noise while do record operation,
most probably it is the headphone standard issue (OMTP and CTIA standard difference). You should use the OMTP
standard headphone. You can disable the left channel by using an headphone without microphone feature.

When the demo runs successfully, you can hear the tone and the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~
SAI example started!
SAI example finished!
 ~~~~~~~~~~~~~~~~~~~
