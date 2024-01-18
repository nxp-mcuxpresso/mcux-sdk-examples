Overview
========
Demonstrates the DMIC working with I2S. One or Stereo channel Audio data is converted to samples in the DMIC module.
Then, the data is placed into the memory buffer. Last, it is send to the I2S buffer and sent
to the CODEC, then the audio data will be output to Lineout of CODEC.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT685 boards
- Personal Computer
- headphones with 3.5 mm stereo jack

Board settings
==============

To make example work, connections needed to be as follows:
  JP7-1        <-->        JP7-2
  JP8-1        <-->        JP8-2

Prepare the Demo
================
1.  Connect headphones to Audio HP / Line-Out connector (J4).
2.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J5) on the board
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
Note: As the EVK-MIMXRT685 support two on board codec, so the demo provide a macro DEMO_DMIC_NUMS to control the counts of the DMIC to be used, such as
#define DEMO_DMIC_NUMS 2U /* two dmic enabled */
If the macro not defined by application, then only one codec will be enabled by default.

1.  Launch the debugger in your IDE to begin running the demo.

The following lines are printed to the serial terminal when the demo program is executed.

Configure codec

Configure I2S

2. This example transfers data from DMIC to Codec. Connect headphone/earphone on audio out of the board.
Speak on DMIC or play song nearby the dmic (U40, U41),  you can hear sound on the left/right channel of headphone/earphone.
