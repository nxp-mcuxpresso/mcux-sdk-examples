Overview
========
The sai_edma_tdm_record_playback example shows how to use sai edma driver receive and playback TDM format data:

In this example, one sai instance reocrd and playbacks the audio data in TDM format.

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- EVKB-MIMXRT1040 board
- CS42448 audio board
- Personal Computer
- Headphone(OMTP standard)
- RCA to 3.5mm line
- Headphone or speaker

Board settings
==============
1.Please mount resistor R480.

2.Please remove the jumper J41.

3.Insert AUDIO board into J23.

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
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
Note:
1.The Stereo input 1 will playback through Line 1&2 output.
2.The Stereo input 2 will playback through Line 3&4 output.
3.The MIC input 1&2 will playback through Line 5&6 output.

~~~~~~~~~~~~~~~~~~~
SAI TDM record playback example started!
Codec Init Done.
~~~~~~~~~~~~~~~~~~~
