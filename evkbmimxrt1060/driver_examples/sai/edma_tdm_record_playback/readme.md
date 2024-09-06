Overview
========
The sai_edma_tdm_record_playback example shows how to use sai edma driver receive and playback TDM format data:

In this example, one sai instance reocrd and playbacks the audio data in TDM format.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- EVKB-MIMXRT1060 board
- Personal Computer
- Headphone or speaker
- CS42448 audio board
- Line in line

Board settings
==============
Insert AUDIO board into J23

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
4.  Insert the CS42448 audio board into J23.
5.  Insert the headphones into the headphone jack of CS42448 audio board line output.
6.  Connect the audio input source to the CS42448 audio board by line in line.
7.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
Note:
The Stereo input 1 will playback through Line 1&2 output.
The Stereo input 2 will playback through Line 3&4 output.
The MIC input 1&2 will playback through Line 5&6 output.

~~~~~~~~~~~~~~~~~~~
SAI TDM record playback example started!
Codec Init Done.
~~~~~~~~~~~~~~~~~~~
