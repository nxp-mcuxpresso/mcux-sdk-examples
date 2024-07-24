Overview
========
The i2s_dma_tdm example shows how to use i2s dma driver with TDM format data:

In this example, one i2s instance playbacks the audio data stored in the sdcard using DMA channel.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- MIMXRT685-AUD-EVK board
- Personal Computer
- headphones with 3.5 mm stereo jack
- sdcard

Board settings
==============
To make example work, please make sure board settings are applied:
  JP43 1-2, R488 1-2
  R479 1-2, R429 1-2
  R480 1-2, R481 1-2
  R510 1-2, R474 1-2

Prepare the Demo
================
1.  Connect headphones to J4 or J50 or J51 or J52.
2.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J5) on the board
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Prepare a FAT32 format sdcard and copy the 8_TDM.wav into sdcard and then Insert the sdcard into J32.
5.  Download the program to the target board.
6.  Launch the debugger in your IDE to begin running the demo.
Running the demo
================
When the demo runs successfully, you can hear

Line 1&2 output: "Front Left", "Front Right"
Line 3&4 output: "Centre", tone
Line 5&6 output: "Back Left", "Back Right"
Line 7&8 output: "Auxiliary Left", "Auxiliary Right"

The log below shows example output of the I2S driver demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
I2S dma TDM example started.



8_TDM.wav File is available



Codec Init Done.



Start play 8_TDM.wav file.



I2S TDM DMA example finished.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

