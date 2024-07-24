Overview
========
The sai_edma_tdm_record_playback example shows how to use sai edma driver receive and playback TDM format data:

In this example, one sai instance reocrd and playbacks the audio data in TDM format.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Micro USB cable
- EVKB-MIMXRT1170 board
- Personal Computer
- Headphone or speaker
- CS42448 audio board
- Line in line

Board settings
==============
1.Please mount resistors below,
R2008,R2022,R2011,R2021,R2009,R2010,R2012,R2016,R1998,R2013,R2014,
R2018,R2017,R2000

2.Remove the resistors below:
R2001,R2002,R2003,R2004,R2005,R2006 and R2007.

3.Insert AUDIO board into J76.

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
Known issue: 
	project: sai_edma_tdm_record_playback@cm4
	compilers: armgcc 
	target: sdram_debug, sdram_release.
	issue: no sound output.
