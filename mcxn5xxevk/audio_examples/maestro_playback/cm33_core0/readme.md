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
- Version: 2.16.000

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Mini/micro USB cable
- MCX-N5XX-EVK board
- Personal Computer
- headphones with 3.5 mm stereo jack

Board settings
==============
Insert the card into the card slot

Prepare the Demo
================
Note: MCUXpresso IDE project default debug console is semihost
1.  Connect headphones to Audio HP / Line-Out connector.
2.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J5) on the board
3.  Open a serial terminal with the following settings :
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

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
