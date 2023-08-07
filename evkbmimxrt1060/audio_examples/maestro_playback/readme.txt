Overview
========
The maestro_demo application demonstrates audio processing on the ARM cortex core
utilizing the Maestro Audio Framework library.

Depending on target platform there are different features of the demo enabled.

    - File decoding and playback
    - EAP effects during file playback
    - Multi-channel playback

The application is controlled by commands from a shell interface using serial console.

Type "help" to see the command list. Similar description will be displayed on serial console:

    >> help

    "help": List all the registered commands

    "exit": Exit program

    "version": Display component versions

    "file": Perform audio file decode and playback

      USAGE: file [start|stop|pause|volume|seek|update|set|get|track|list|info]
        start             Play default (first found) or specified audio track file.
        stop              Stops actual playback.
        pause             Pause actual track or resume if already paused.
        volume=<volume>   Set volume. The volume can be set from 0 to 100.
        seek=<seek_time>  Seek currently paused track. Seek time is absolute time in milliseconds.
        update=<preset>   Apply current EAP parameters without attribute value
                          or switch to preset 1-10
        set=<preset>      Apply current EAP parameters without attribute value
                          or switch to preset 1-10
        get               Sync actual EAP parameters from library to ATT config structures.
        track=<filename>  Select audio track to play.
        list              List audio files available on mounted SD card.
        info              Prints playback info.


Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- JTAG/SWD debugger
- MIMXRT1060-EVKB board
- Personal Computer
- Headphones with 3.5 mm stereo jack
- Audio expansion board AUD-EXP-42448 (REV B)

Board settings
==============
Make sure resistors R368/R347/R349/R365/R363 are removed to be able to use SD-Card.

For Audio board:
    1.Insert AUDIO expansion board into J23 to be able to use the CS42448 codec for multichannel output.
    2.Uninstall J41
    3.Define DEMO_CODEC_CS42448 1 in app_definitions.h
For on board codec:
    1.Make sure J41 is installed
    2.Define DEMO_CODEC_WM8960 1 in app_definitions.h

Prepare the Demo
================
- As the EVKBMIMXRT1060 support two codecs, a default on board WM8960 codec and another codec CS42448 on audio board, so to support both of the codecs, the example provide options to switch between the two codecs.
  The options are located in the app_definitions.h file.
    - DEMO_CODEC_WM8960, set to 1 if wm8960 used
    - DEMO_CODEC_CS42448, set to 1 if cs42448 used
    Please do not set above macros to 1 together, as the demo support one codec only.

- This development board also supports multi-channel example. The example demonstrates playback of raw PCM files from an SD-card with up to 8 channels, 96kHz sample rate and 32 bit width.
    - To enable multi-channel example:
        1. Connect the Audio board to the development board
        2. Define the MULTICHANNEL_EXAMPLE macro in the project settings
        3. Set the DEMO_CODEC_CS42448 macro to 1 in the app_definitions.h file
        4. Remove the EAP16 library from the MCU Linker Libraries

1.  Connect a micro USB cable between the PC host and the debug USB port on the board
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
5. Insert the headphones into the headphone jack J6 and line in line into J12 on the audio board.
6. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Note:
- For the full output range of the EAP crossover options when applied to a stereo audio file, define XO_USE_FULL_STEREO in the eap_proc.h file
    - When the full output range is enabled, then the cs42448 codec must also be enabled and Audio board must be connected to the development board.
    - When the full output range is disabled and the input audio file is stereo, then the example sends only the low and high band of the left channel crossover output to the Line-Out port
      (this means that the output signal from the example is also stereo). The right channel of the crossover output is ignored in this case.
- To enable FLAC decoding and playback it is necessary to disable EAP:
   1. Define FLAC_DEC=1 in the project settings
   2. Undefine EAP_PROC in the project settings

- The AAC decoder is only supported in MCUXpresso and ARMGCC.

Running the demo
================
When the example runs successfully, you should see similar output on the serial terminal as below:

*********************************
Maestro audio playback demo start
*********************************

[APP_Main_Task] started

Copyright  2022  NXP
[APP_SDCARD_Task] start
[APP_Shell_Task] start

>> [APP_SDCARD_Task] SD card drive mounted

Known issues

1. MP3 decoder has issues with some of the files. One of the channels can be sometimes distorted or missing parts of the signal.
2. When using EAP crossover feature together with SSRC, after finishing the playback, it might be necessary to reset
   the board in order to have correct sound output. Otherwise the sound output may be distorted.
