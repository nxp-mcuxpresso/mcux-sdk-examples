Overview
========
The sai_interrupt_transfer example shows how to use sai driver with interrupt:

In this example, one sai instance playbacks the audio data stored in flash/SRAM using interrupt.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

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
4. Insert the headphones into the headphone jack J6 on the audio board.
5. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
note: This demo uses 44.1KHZ sample rate to play a wav music.

When the demo runs successfully, you can hear the tone and the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~
SAI example started!
SAI example finished!
~~~~~~~~~~~~~~~~~~~
