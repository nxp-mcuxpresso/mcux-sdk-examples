Overview
========
The asrc m2m interrupt example shows how to use asrc driver with interrupt:

In this example, asrc will convert the audio data and playback the through SAI.

Toolchain supported
===================
- MCUXpresso  11.6.0
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1160-EVK board
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
4. Insert the headphones into the headphone jack on MIMXRT1160-EVK board (J33).
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
