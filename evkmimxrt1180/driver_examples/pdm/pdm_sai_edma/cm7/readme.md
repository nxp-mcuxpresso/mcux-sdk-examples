Overview
========
The pdm sai edma example shows how to use pdm driver with edma:

In this example, pdm gathers one channel's audio data though edma, then sai sends it to codec.

SDK version
===========
- Version: 2.15.001

Toolchain supported
===================
- GCC ARM Embedded  12.3.1
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1180-EVK board
- Personal Computer
- Headphone

Board settings
==============
Insert headphone to J101.

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
4.  Press the reset button on your board to begin running the demo.

Note
The CM7 demo is supposed to work with multicore_trigger CM33 demo in this SDK.

Running the demo
================
When the demo runs successfully, you can hear the sound and the log would be seen on the OpenSDA terminal like:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PDM SAI Edma example started!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
