Overview
========
The sai_edma_ping_pong_buffer example shows how to use sai driver with EDMA scatter gather feature to implement ping pong buffer case.

In this example, one sai instance playbacks the audio data received from external codec.

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
- Headphone(OMTP standard)
- CS42448 Audio board(Not necessary if use on board codec)
- Power adapter

Board settings
==============
For on board codec:
1.Make sure J97/J98/J99/J100 is installed

For cs42448 audio board:
1.Remove Jumper J99 and remove R1084 resistor.
(If use MCU-LINK JLINK firmware, please also remove R82 resistor)

2.Insert AUDIO board into J47.

3. Switch Jumper J1 to 1-2, insert power adapter on J2 and power on the board by SW1.
(Since the power consumption exceeds the USB power supply capacity when using CS42448 audio board, an external power supply must be used)

Prepare the Demo
================
Note: As the MIMXRT1180-EVK support two codecs, a default on board WM8962 codec and another codec CS42448 on audio board, so to support both of the codecs, the example provide options to switch between the two codecs,
- DEMO_CODEC_WM8962, set to 1 if wm8962 used
- DEMO_CODEC_CS42448, set to 1 if cs42448 used
Please do not set above macros to 1 together, as the demo support one codec only.

1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
Steps for WM8962:
4. Insert the headphones into the headphone jack on MIMXRT1170-EVKB board (J101).
Steps for CS42448:
4. Insert the headphones into the headphone jack J6 and line in line into J12 on the audio board.
5. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
Note: This demo uses both headphone mic and board main mic(P2) as input source of WM8962. The headphone mic provides left
channel data, and main mic (P2) provides right channel data. If users found there is noise while do record operation,
most probably it is the headphone standard issue (OMTP and CTIA standard difference). You should use the OMTP
standard headphone. You can disable the left channel by using an headphone without microphone feature.
This demo outputs 1KHZ sine wave audio signal.

When the demo runs successfully, you can hear the tone and the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~
SAI EDMA ping pong buffer example started!
~~~~~~~~~~~~~~~~~~~
