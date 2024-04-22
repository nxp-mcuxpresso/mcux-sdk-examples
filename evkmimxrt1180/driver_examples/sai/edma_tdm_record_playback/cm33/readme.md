Overview
========
The sai_edma_tdm_record_playback example shows how to use sai edma driver receive and playback TDM format data:

In this example, one sai instance reocrd and playbacks the audio data in TDM format.

SDK version
===========
- Version: 2.15.001

Toolchain supported
===================
- GCC ARM Embedded  12.3.1
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Micro USB cable
- MIMXRT1180-EVK board
- Personal Computer
- Headphone or speaker
- CS42448 audio board
- Line in line
- Power adapter

Board settings
==============
1.Remove Jumper J99 and remove R1084 resistor.
(If use MCU-LINK JLINK firmware, please also remove R82 resistor)

2.Insert AUDIO board into J47.

3. Switch Jumper J1 to 1-2, insert power adapter on J2 and power on the board by SW1.
(Since the power consumption exceeds the USB power supply capacity when using CS42448 audio board, an external power supply must be used)

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
4.  Insert the CS42448 audio board into J76.
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
