Overview
========
The pdm sai multi channel tdm edma example shows how to use pdm driver with edma in tdm mode:

In this example, pdm gathers eight channels' audio data though edma, then sai sends it to codec.

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVKB board
- Personal Computer
- 8-DMIC board
- CS42448 audio board
- Headphone

Board settings
==============
MIMXRT1170-EVKB:
1. To enable external DMIC (J78), remove R2035, R2038.
2. To support CS42448 audio board, weld R1998, R2008, R2009, R2011, R2010, R2012, R2013, R2014, R2000, R2018, R2017, R2016, R2022
3. After running the demo. If output 3/4 or 7/8 has no sound,you should remove the J112.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2   Insert the CS42448 audio board into J76.
3   Insert the 8-DMIC board into J78.
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
When the demo runs successfully, if output 3/4 has no sound, remove the J112 and then you can hear the sound from microphone(8-DMIC board)
and the log would be seen on the OpenSDA terminal like:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
PDM SAI multi channel TDM edma example started!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
User can press the key SW7 to adjust the MIC output gain, the gain value will be looped in the rang 0-15.
