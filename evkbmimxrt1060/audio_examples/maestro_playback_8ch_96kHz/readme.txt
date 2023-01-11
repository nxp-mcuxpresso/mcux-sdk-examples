Overview
========
This application demonstrates audio processing on the ARM cortex core
utilizing the Maestro Audio Framework library. This particular example demonstrates playback of
raw pcm files from sd-card with up to 8 channels, 96kHz sample rate and 32 bit width.

Depending on target platform there are different features of the demo enabled.

    - file decoding and playback
    - EAP effects during file playback

The application is controlled by commands from a shell interface using serial console.

Type "help" to see the command list. Similar description will be displayed on serial console:

    >> help

    "help": List all the registered commands

    "exit": Exit program

    "version": Display component versions

    "file": Perform audio file decode and playback

      USAGE: file [start|stop|pause|update|track|list|info]
        start             Play default (first found) or specified audio track file.
        stop              Stops actual playback.
        pause             Pause actual track or resume if already paused.
        update=<preset>   Apply current EAP parameters without attribute value
                          or switch to preset 1-10
        set=<preset>      Apply current EAP parameters without attribute value
                          or switch to preset 1-10
        get               Sync actual EAP parameters from library to ATT config structures.
        track=<filename>[num_channels]   Select audio track to play. Select 2 or 8 channels.
                                         - If channel number not specified, default 8 is used.
        list              List audio files available on mounted SD card.
        info              Prints playback info.
      NOTE: Selected audio track must always meet the following parameters:
                      - Sample rate:        96 kHz
                      - Width:              32 bit
                      - Number of channels: Depending on the [num_channels] parameter
      NOTE: Only when 2 channels are selected EAP can be applied to the audio track.


Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Micro USB cable
- JTAG/SWD debugger
- MIMXRT1060-EVKB board
- Personal Computer
- Headphones with 3.5 mm stereo jack / multi channel speakers
- Audio expansion board AUD-EXP-42448 (REV B)

Board settings
==============
Make sure resistors R368/R347/R349/R365/R363 are removed to be able to use SD-Card.

Insert AUDIO expansion board into J23 to be able to use the CS42448 codec for multichannel output.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the debug USB port (J41) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4. Insert the headphones into the headphone jack J6. If playing multiple channels, please insert multiple line-output
   or headphones in line-ouptut jack on the audio extension board.
5. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.
Running the demo
================
When the example runs successfully, you should see similar output on the serial terminal as below:

**********************************
Maestro audio solutions demo start
**********************************

[APP_SDCARD_Task] start
[APP_Shell_Task] start

SHELL build: Nov  5 2020
Copyright  2020  NXP

>> [APP_SDCARD_Task] SD card drive mounted
