Overview
========
The sai_edma_tdm example shows how to use sai edma driver with TDM format data:

In this example, one sai instance playbacks the audio data stored in the sdcard using EDMA channel.

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
- MIMXRT1180-EVK board
- Personal Computer
- Headphone or speaker
- SDCARD
- CS42448 audio board
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
6.  Prepare a FAT32 format sdcard and copy the 8_TDM.wav into sdcard and then Insert the sdcard into J15.
7.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs successfully, you can hear

Line 1&2 output: "Front Left", "Front Right"
Line 3&4 output: "Centre", tone
Line 5&6 output: "Back Left", "Back Right"
Line 7&8 output: "Auxiliary Left", "Auxiliary Right"

and the log would be seen on the OpenSDA terminal like:
~~~~~~~~~~~~~~~~~~~
SAI edma TDM example started.

8_TDM.wav File is available

Codec Init Done.

Start play 8_TDM.wav file.

SAI TDM EDMA example finished.
~~~~~~~~~~~~~~~~~~~
