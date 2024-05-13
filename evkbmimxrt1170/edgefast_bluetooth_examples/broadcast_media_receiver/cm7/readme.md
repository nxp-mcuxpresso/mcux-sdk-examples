Overview
========
Application demonstrating how to use the broadcast media receiver feature.

There should be three boards: 1 BMS + 1 BMR(left) + 1 BMR(right).
BMS: broadcast stereo audio stream, left channel on first BIS and right channel on another BIS.
BMR: lisen one of BIS channel and render it.


SDK version
===========
- Version: 2.15.100

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- evkbmimxrt1170 board
- Personal Computer
- Embedded Artists 2EL M.2 Module (Rev-A1) - direct M2 connection.

Jumper settings for RT1170-EVKB (enables external 5V supply):
remove  J38 5-6
connect J38 1-2
connect J43 with external power(controlled by SW5)
connect J25-15 with J97
connect J25-13 with 2EL's GPIO_27

Murata Solution Board settings
Embedded Artists 2EL module datasheet: https://www.embeddedartists.com/doc/ds/2EL_M2_Datasheet.pdf

The hardware should be reworked according to the Hardware Rework Guide for MIMXRT1170-EVKB and Murata 1XK M.2 Adapter in document Hardware Rework Guide for EdgeFast BT PAL.
The hardware rework for MIMXRT1170-EVKB and Murata 2EL M.2 Adapter is same as MIMXRT1170-EVKB and Murata 1XK M.2 Adapter.

Note:
To ensure that the LITTLEFS flash region has been cleaned,
all flash sectors need to be erased before downloading example code.
After downloaded binary into qspiflash and boot from qspiflash directly,
please reset the board by pressing SW4 or power off and on the board to run the application.
Prepare the Demo
================

1.  Open example's project and build it.

2.  Connect a USB cable between the PC host and the OpenSDA USB port on the target board.

3.  Provide 5V voltage for the target board.

4.  Connect speaker to Audio Jack.

5.  Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control

6.  Download the program to the target board.

7.  Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
The log below shows the output of the example in the terminal window.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Copyright  2022  NXP

BMR>> 
Broadcast Media Receiver.

Please select sink role "left"|"right" use "init" command.

BMR>> init left

BMR@left>> BMR@left>> Bluetooth initialized
Scanning for broadcast sources

[device name]:broadcast_media_sender
connect...
Broadcast source found, waiting for PA sync
PA synced for broadcast sink 202DBE48 with broadcast ID 0xD789F0
Received BASE with 1 subgroups from broadcast sink 202DBE48
Broadcast source PA synced, waiting for BASE
BASE received, waiting for syncable
	Codec: freq 16000, channel count 1, duration 10000, channel alloc 0x00000001, frame len 40, frame blocks per sdu 1
Audio codec configed, waiting for syncable
Syncing to broadcast
Stream 202E3164 started
BMR@left>> 
BMR@left>> 
BMR@left>> 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Procedures to run
1 input "init left" or "init right" to start sink role and start to scan broadcast media sender, audio will start playing after sync to source.
2 input "pause" to stop playing.
3 input "play" to start playing.
4 input "vol_up", "vol_down", "vol_set" to set volume.
5 input "vol_mute", "vol_unmute" to set mute.

Note:
1 "exit" command is a shell internal command, only used to exit shell module and could not used to exit demo.
