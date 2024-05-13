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

      USAGE: file [start|stop|pause|volume|seek|track|list|info]
        start             Play default (first found) or specified audio track file.
        stop              Stops actual playback.
        pause             Pause actual track or resume if already paused.
        volume=<volume>   Set volume. The volume can be set from 0 to 100.
        seek=<seek_time>  Seek currently paused track. Seek time is absolute time in milliseconds.
        track=<filename>  Select audio track to play.
        list              List audio files available on mounted SD card.
        info              Prints playback info.
```


SDK version
===========
- Version: 2.15.100

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- LPCXpresso55s69 board
- Personal Computer
- headphones with 3.5 mm stereo jack
- source of sound (line output to 3.5 mm stereo jack)

Board settings
==============
Insert the card into the card slot

Prepare the Demo
================
1. Connect headphones to Audio HP / Line-Out connector.
2. Connect source of sound to Audio Line-In connector.
3. Connect a micro USB cable between the PC host and the CMSIS DAP USB port (P6) on the board
4. Open a serial terminal with the following settings :
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
5. Download the program to the target board.
6. Either press the reset button on your board or launch the debugger in your IDE to begin
   running the demo.

### Notes
- MCUXpresso IDE project default debug console is semihost
- OPUS decoder is disabled due to insufficient memory.
- The AAC decoder is only supported in MCUXpresso and ARMGCC.
- When playing FLAC audio files with too small frame size (block size), the audio output
  may be distorted because the board is not fast enough.
- When a memory allocation ERROR occurs, it is necessary disable the SSRC element due to
  insufficient memory.

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
2. When running the "file start" command during playback, the example fails. It is necessary to stop or pause the playback first.
