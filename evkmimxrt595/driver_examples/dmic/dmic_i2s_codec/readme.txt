Overview
========
Demonstrates the DMIC working with I2S. Audio is converted to samples in the DMIC module.
Then, the data is placed into the I2S buffer directly without DMA or CPU intervention. Last, it is read from the I2S buffer and sent
to the CODEC, then the audio data will be output to Lineout of CODEC.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.4.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT595 boards
- Personal Computer
- headphones with 3.5 mm stereo jack

Board settings
==============
To make example work, connections needed to be as follows:
The 1-2 connected for JP7, JP8, JP27, JP28, JP29.

Prepare the Demo
================
1.  Connect headphones to Audio HP / Line-Out connector (J4).
2.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port on the board
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
1.  Launch the debugger in your IDE to begin running the demo.
No log will be printed in this demo, since the FC0 DATA pin is mux as the I2S0(only I2S0 support PDM data feature)

2. This example transfers data from DMIC to Codec. Connect headphone/earphone on audio out of the board.
Speak on DMIC or play song nearby the dmic (U120 and U121),  you can hear sound on the left channel of headphone/earphone.
