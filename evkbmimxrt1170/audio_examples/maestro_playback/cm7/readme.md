Overview
========
The maestro_playback application demonstrates audio processing on the ARM cortex core
utilizing the Maestro Audio Framework library.

Depending on target platform there are different features of the demo enabled.

    - File decoding and playback
    - Multi-channel playback

The application is controlled by commands from a shell interface using serial console.

```
Type "help" to see the command list. Similar description will be displayed on serial console:

    >> help

    "help": List all the registered commands

    "exit": Exit program

    "version": Display component versions

    "file": Perform audio file decode and playback

      USAGE: file [stop|pause|volume|seek|play|list|info]
        stop              Stops actual playback.
        pause             Pause actual track or resume if already paused.
        volume=<volume>   Set volume. The volume can be set from 0 to 100.
        seek=<seek_time>  Seek currently paused track. Seek time is absolute time in milliseconds.
        play=<filename>   Select audio track to play.
        list              List audio files available on mounted SD card.
        info              Prints playback info.
```


SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- JTAG/SWD debugger
- EVKB-MIMXRT1170 board
- Personal Computer
- Headphones with 3.5 mm stereo jack
- SD card

Board settings
==============
To make the examples work:
1.  Please remove below resistors if on board wifi chip is not DNP:
    - R228
    - R229
    - R232
    - R234
2.  Please make sure R136 is weld for GPIO card detect.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the debug USB port (J86) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Connect the headphones into the headphone jack on EVKB-MIMXRT1170 board (J101).
5.  Either press the reset button on your board or launch the debugger in your IDE to begin
    running the demo.

### Notes
- The AAC decoder is only supported in MCUXpresso and ARMGCC.

Running the demo
================
When the example runs successfully, you should see similar output on the serial
terminal as below:
```
    *********************************
    Maestro audio playback demo start
    *********************************

    [APP_Main_Task] started

    Copyright  2022  NXP
    [APP_SDCARD_Task] start
    [APP_Shell_Task] start

    >> [APP_SDCARD_Task] SD card drive mounted
```

# Known issues
1. MP3 decoder has issues with some of the files. One of the channels can be sometimes
   distorted or missing parts of the signal.
2. Playback is missing a fraction of second in the beginning and end of the stream.
