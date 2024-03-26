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
```
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
```

For custom VIT model generation (defining own wake words and voice commands) please
use https://vit.nxp.com/

### Notes
1.  VIT and VoiceSeeker libraries are only supported in the MCUXpresso IDE.
2.  If more than one channel is used and VIT is enabled, please enable VoiceSeeker.
    - The VoiceSeeker that combines multiple channels into one must be used, as VIT can
    only work with one channel.


SDK version
===========
- Version: 2.15.100

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- JTAG/SWD debugger
- MIMXRT1060-EVKC board
- Personal Computer
- Headphones with 3.5 mm stereo jack
- Audio expansion board AUD-EXP-42448 (REV B)

Board settings
==============
For Audio expansion board:
1. Insert AUDIO board into J19 if on board codec is not used
2. Uninstall J99
3. Define DEMO_CODEC_CS42448 1 in app_definitions.h

For on board codec:
1. Make sure J99 is installed
2. Define DEMO_CODEC_WM8962 1 in app_definitions.h

Prepare the Demo
================
### Macros settings
- EVKCMIMXRT1060 supports two codecs. Default on board WM8962 codec and additional
codec CS42448 on audio board. To support both of the codecs, the example provides options
to switch between them using macros, located in app_definitions.h:
    - DEMO_CODEC_WM8962, set to 1 if wm8962 is used (on board codec)
    - DEMO_CODEC_CS42448, set to 1 if cs42448 is used (audio board codec)
    Please do not set both macros to 1 together, as the demo supports using one codec at a time.

- In order to enable VoiceSeeker it is necessary to add VOICE_SEEKER_PROC to preprocessor defines
on project level and either connect the AUD-EXP board or provide another multi-channel data to
VoiceSeeker.

### Procedure
1. Connect a micro USB cable between the PC host and the debug USB port on the board
2. Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Download the program to the target board.
Steps for WM8962:
4. Insert the headphones into the headphone jack on MIMXRT1060-EVKC board (J101).
Steps for CS42448:
4. For the loopback (record_mic audio) and for the file output (record_mic file) the audio
   stream is as follows:
    - Stereo INPUT 1 (J12) -> LINE 1&2 OUTPUT (J6)
    - Stereo INPUT 2 (J15) -> LINE 3&4 OUTPUT (J7)
    - MIC1 & MIC2 (P1, P2) -> LINE 5&6 OUTPUT (J8)
    - Insert the headphones into the different line outputs to hear the inputs.
    - To use the Stereo INPUT 1, 2, connect an audio source LINE IN jack.
    - Please have in mind that the resulting pcm file in case of the file output has following
      parameters:
    - WM8962 codec:  2 channels, 16kHz, 16bit width
    - CS42448 codec: 8 channels, 16kHz, 32bit width
5. Either press the reset button on your board or launch the debugger in your IDE to begin
   running the demo.

Running the demo
================
When the example runs successfully, you should see similar output on the serial terminal as below:
```
    *******************************
    Maestro audio record demo start
    *******************************

    Copyright  2022  NXP
    [APP_SDCARD_Task] start
    [APP_Shell_Task] start

    >> [APP_SDCARD_Task] SD card drive mounted
```

