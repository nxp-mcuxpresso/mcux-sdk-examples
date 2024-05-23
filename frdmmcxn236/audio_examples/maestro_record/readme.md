Overview
========
The maestro_record application demonstrates audio processing on the ARM cortex core
utilizing the Maestro Audio Framework library.

Depending on target platform there are different features of the demo enabled.

    - Loopback from microphone to speaker
    - Recording microphone to a file
    - Wake words + voice commands recognition

The application is controlled by commands from a shell interface using serial console.

Type "help" to see the command list. Similar description will be displayed on serial console:

    >> help

    "help": List all the registered commands

    "exit": Exit program

    "version": Display component versions

    "record_mic": Record MIC audio and either:
    - perform voice recognition (VIT)
    - playback on codec
    - store samples to file.

    USAGE: record_mic [audio|file|<file_name>|vit] 20 [<language>]
    The number defines length of recording in seconds.
    Please see the project defined symbols for the languages supported.
    Then specify one of: en/cn/de/es/fr/it/ja/ko/tr as the language parameter.
    For voice recognition say supported WakeWord and in 3s frame supported command.
    Please note that this VIT demo is near-field and uses 1 on-board microphone.
    To store samples to a file, the "file" option can be used to create a file
    with a predefined name, or any file name (without whitespaces) can be specified
    instead of the "file" option.
    This command returns to shell after the recording is finished.

    "opus_encode": Initializes the streamer with the Opus memory-to-memory pipeline and
    encodes a hardcoded buffer.

For custom VIT model generation (defining own wake words and voice commands) please use https://vit.nxp.com/

Notes:
    - VIT and VoiceSeeker libraries are only supported in the MCUXpresso IDE.
    - If more than one channel is used and VIT is enabled, please enable VoiceSeeker.
        - The VoiceSeeker that combines multiple channels into one must be used, as VIT can only work with one channel.


SDK version
===========
- Version: 2.14.0

Toolchain supported
===================
- MCUXpresso  11.9.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXN236 board
- Personal Computer
- headphones with 3.5 mm stereo jack

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the MCU-Link USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs successfully, you should see similar output on the serial terminal as below:

    *******************************
    Maestro audio record demo start
    *******************************

    Copyright  2022  NXP
    [APP_SDCARD_Task] start
    [APP_Shell_Task] start

    >> [APP_SDCARD_Task] SD card drive mounted
