Overview
========
The sai_interrupt_record_playback example shows how to use sai driver with record and playback features:

In this example, one sai instance record the audio data from input and playbacks the audio data.

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1060-EVKB board
- Personal Computer
- Headphone(OMTP standard)
- CS42448 Audio board(Not necessary if use on board codec)

Board settings
==============
For Audio board:
1.Insert AUDIO board into J23 if on board codec is not used
2.Uninstall J41
For on board codec:
2.Make sure J41 is installed

Prepare the Demo
================
Note: As the EVKBMIMXRT1060 support two codecs, a default on board WM8960 codec and another codec CS42448 on audio board, so to support both of the codecs, the example provide options to switch between the two codecs,
- DEMO_CODEC_WM8960, set to 1 if wm8960 used
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
Steps for WM8960:
4. Insert the headphones into the headphone jack on MIMXRT1060-EVKB board (J34).
Steps for CS42448:
4. Insert the headphones into the headphone jack J6 and line in line into J12 on the audio board.
5. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================

Note: This demo uses both headphone mic and board main mic(P1) as input source of WM8960. The headphone mic provides left
channel data, and main mic (P1) provides right channel data. If users found there is noise while do record operation,
most probably it is the headphone standard issue (OMTP and CTIA standard difference). You should use the OMTP
standard headphone. You can disable the left channel by using an headphone without microphone feature.

When the demo runs successfully, you can hear the tone and the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~
SAI example started!
SAI example finished!
~~~~~~~~~~~~~~~~~~~
