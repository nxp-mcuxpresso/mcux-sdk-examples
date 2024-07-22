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
- Mini/micro USB cable
- EVKB-MIMXRT1040 board
- Personal Computer
- Headphone(OMTP standard)
- SDCARD
- CS42448 audio board
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
