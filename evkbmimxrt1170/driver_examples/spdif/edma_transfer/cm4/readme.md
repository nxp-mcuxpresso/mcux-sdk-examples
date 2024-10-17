Overview
========
The spdif_edma_transfer example shows how to use spdif driver with edma:

In this example, one spdif instance playbacks the audio data recorded by same spdif instance using edma.

Notice: Please use 48KHz sample rate for PC playback and set SPDIF output device to 48KHz.

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
- Two SPDIF RCA lines
- Soundcacrd support SPDIF interface

Board settings
==============
1.Make sure J35,J36,J37,J45,J46,T1 and T2 are installed. Weld 10kΩ resistor to R295.
2.Remove R188, R203, R209 and R835.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Connect J46 with SPDIF soundcard output inteface, connect J45 with SPDIF soundcard input interface.
3.  Playback music using the SPDIF soundcard, make the SPDIF signals input to MIMXRT1170 board.
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
After run the demo, you can hear the music playbacked from SPDIF soundcacrd, the log below shows the output of the example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SPDIF EDMA example started!
SPDIF EDMA example finished!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# Known issues
1. When running a flash target of the demo, it gets blocked after printing the "SPDIF EDMA example started!".
