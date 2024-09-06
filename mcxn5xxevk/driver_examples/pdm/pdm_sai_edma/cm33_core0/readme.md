Overview
========
The pdm sai edma example shows how to use pdm driver with edma:

In this example, pdm gathers one channel's audio data though edma, then sai sends it to codec.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- MCX-N5XX-EVK board
- Personal Computer
- Headphone

Board settings
==============
Short JP7 2-3, JP8 2-3, JP10 2-3, JP11 2-3

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the MCU-Link USB port on the target board.
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
When the demo runs successfully, you can hear the sound from microphone(U30) and the log would be seen on the MCU-Link terminal like:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PDM SAI interrupt transfer example started!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
