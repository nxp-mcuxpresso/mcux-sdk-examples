Overview
========
The pdm sai multi channel edma example shows how to use pdm driver with edma:

In this example, pdm gathers two channels' audio data though edma, then sai sends it to codec, the received DMIC data format,
 ----------------------------------------------------------------------------------------------------------------------
 |CHANNEL0 | CHANNEL1 | CHANNEL2 | .... | CHANNEL0 | CHANNEL 1 | CHANNEL2 |....| CHANNEL0 | CHANNEL 1 | CHANNEL2 |....|
 ----------------------------------------------------------------------------------------------------------------------

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

Running the demo
================
When the demo runs successfully, you can hear the sound and the log would be seen on the OpenSDA terminal like:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PDM SAI multi channel edma example started!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
