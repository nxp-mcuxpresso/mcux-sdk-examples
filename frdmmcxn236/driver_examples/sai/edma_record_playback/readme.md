Overview
========
The sai_edma_record_playback example shows how to use sai driver with EDMA:

In this example, one sai instance record the audio data from input and playbacks the audio data.

SDK version
===========
- Version: 2.14.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXN236 board
- Personal Computer
- Headphone(OMTP standard)
- Line-in line

Board settings
==============
No special settings are required.

Prepare the Demo
================
1. Connect a USB cable between the host PC and the MCU-Link USB port on the target board.
2. Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4. Insert the headphones into the headphone jack on board (J14).
5. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs successfully, you can hear the tone and the log would be seen on the MCU-Link terminal like:

~~~~~~~~~~~~~~~~~~~
SAI example started!
~~~~~~~~~~~~~~~~~~~
