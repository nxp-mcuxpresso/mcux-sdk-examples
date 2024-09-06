Overview
========
The asrc m2m interrupt example shows how to use asrc driver with interrupt:

In this example, asrc will convert the audio data and playback the through SAI.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVKB board
- Personal Computer
- Headphone(OMTP standard)

Board settings
==============
No special settings are required.

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
4. Insert the headphones into the headphone jack on MIMXRT1170-EVKB board (J101).
5. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs successfully, you can hear the tone and the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~
ASRC m2m edma example

Playback raw audio data

    sample rate : xxxx

    channel number: xxxx

    frequency: xxxx.


Playback converted audio data

    sample rate : xxxx

    channel number: xxxx

    frequency: xxxx.

ASRC m2m edma example finished
 ~~~~~~~~~~~~~~~~~~~
