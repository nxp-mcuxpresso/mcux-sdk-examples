Overview
========
The maestro_sync application demonstrates the use of synchronous pipelines (Tx and Rx in this case)
processing on the ARM cortex core utilizing the Maestro Audio Framework library.
This feature is useful for testing the latency of the pipeline or implementing
algorithms requiring reference signals (like echo cancellation). The libraries
available in this example (VoiceSeeker) are not featuring AEC (acoustic echo cancellation),
but NXP is offering it in the premium version of the libraries. You can visit www.nxp.com/voiceseeker
for more information.

The demo uses two pipeline running synchronously in a single streamer task:
    1. Playback pipeline:
        - Playback of audio data in PCM format stored in flash memory to the speaker.
    2. Recording pipline:
        - Record audio data using a microphone.
        - VoiceSeeker processing.
        - Wake words + voice commands recognition.
        - Save VoiceSeeker output to SD card.

The application is controlled by commands from a shell interface using serial console.

Type "help" to see the command list. Similar description will be displayed on serial console:

    >> help

    "help": List all the registered commands

    "exit": Exit program

    "version": Display component versions

    "start [nosdcard]": Starts a streamer task.
                  - Initializes the streamer with the Memory->Speaker pipeline and with
                    the Microphone->VoiceSeeker->VIT->SDcard pipeline.
                  - Runs repeatedly until stop command.
         nosdcard - Doesn't use SD card to store data.

    "stop": Stops a running streamer:

    "debug [on|off]": Starts / stops debugging.
                    - Starts / stops saving VoiceSeeker input data (reference and microphone data) to SDRAM.
                    - After the stop command, this data is overwritten to the SD card.

For custom VIT model generation (defining own wake words and voice commands) please use https://vit.nxp.com/


Toolchain supported
===================
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- JTAG/SWD debugger
- MIMXRT1060-EVKB board
- Personal Computer
- Speaker with 3.5 mm stereo jack
- SD card

Board settings
==============
- Make sure resistors R368/R347/R349/R365/R363 are removed to be able to use SD-Card.
- Make sure J41 is installed.
- Please insert the SDCARD into the card slot in order to record to a VoiceSeeker output.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the debug USB port on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Download the program to the target board.
4. Connect the speaker into the line out jack on MIMXRT1060-EVKB board (J34).
5. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs successfully, you should see similar output on the serial terminal as below:

*****************************
Maestro audio sync demo start
*****************************

Copyright  2022  NXP
[APP_SDCARD_Task] start
[APP_Shell_Task] start

>> [APP_SDCARD_Task] SD card drive mounted
