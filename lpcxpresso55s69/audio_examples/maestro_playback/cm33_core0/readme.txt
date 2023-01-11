Overview
========
The maestro_demo application demonstrates audio processing on the ARM cortex core
utilizing the Maestro Audio Framework library.

Depending on target platform there are different features of the demo enabled.

    - File decoding and playback
    - EAP effects during file playback

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
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

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
Note: MCUXpresso IDE project default debug console is semihost
1.  Connect headphones to Audio HP / Line-Out connector.
2.  Connect source of sound to Audio Line-In connector.
3.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (P6) on the board
4.  Open a serial terminal with the following settings :
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
5.  Download the program to the target board.
6.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Note:
There is limited RAM on this platform, which brings following limitations:
 - EAP is enabled just with mp3 files
 - OPUS decoder is disabled due to insufficient memory
 - To enable AAC decoding and playback it is necessary to disable EAP:
    1. Define AAC_DEC=1 in the project settings
    2. Undefine EAP_PROC in the project settings
 - To enable WAV decoding and playback it is necessary to disable EAP:
    1. Define WAV_DEC=1 in the project settings
    2. Undefine EAP_PROC in the project settings
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
