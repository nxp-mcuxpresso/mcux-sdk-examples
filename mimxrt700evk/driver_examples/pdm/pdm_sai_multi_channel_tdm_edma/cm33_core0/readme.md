Overview
========
The pdm sai multi channel tdm edma example shows how to use pdm driver with edma in tdm mode:

In this example, pdm gathers eight channels' audio data though edma, then sai sends it to codec.

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
- MIMXRT700-EVK board
- Personal Computer
- 8CH-DMIC board
- AUD-EXP-42448 audio board
- Headphone

Board settings
==============
1. To enable external DMIC (J7), remove R493.
Connect AUD-EXP-42448 audio board to J21.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the MCU-LINK USB port on the target board.
2   Insert the CS42448 audio board into J21.
3   Insert the 8-DMIC board into J7.
4.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
5.  Download the program to the target board.
6.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log would be seen on theterminal like:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PDM SAI multi channel TDM edma example started!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
