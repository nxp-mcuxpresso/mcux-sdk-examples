Overview
========
Application demonstrating how to use the broadcast media sender feature.

There should be three boards: 1 BMS + 1 BMR(left) + 1 BMR(right).
BMS: broadcast stereo audio stream, left channel on first BIS and right channel on another BIS.
BMR: lisen one of BIS channel and render it.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

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

4.  Save a wav music file to a USB disk's root directory and named as "music_16_2.wav".

5.  Connect the USB disk to USB OTG port.

6.  Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control

7.  Download the program to the target board.

8.  Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
The log below shows the output of the example in the terminal window.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Bluetooth initialized

wav file list:
1, 1:/._music_8000_2ch_16bits.wav
2, 1:/music_48000_2ch_16bits.wav
3, 1:/._music_16_2.wav
4, 1:/music_44100_2ch_16bits.wav
5, 1:/music_32000_2ch_16bits.wav
6, 1:/play_1ksin_48k_16b.wav
7, 1:/music_24000_2ch_16bits.wav
8, 1:/music_16000_2ch_16bits.wav
9, 1:/music_8000_2ch_16bits.wav
10, 1:/music_16_2.wav
11, 1:/sine_16_2.wav
12, 1:/sine_8000_2ch_16bits.wav
13, 1:/sine_16000_2ch_16bits.wav
14, 1:/sine_24000_2ch_16bits.wav
15, 1:/sine_32000_2ch_16bits.wav
16, 1:/sine_44100_2ch_16bits.wav
17, 1:/sine_48000_2ch_16bits.wav
18, 1:/sine_16000_2ch_32bits.wav
19, 1:/sine_16000_2ch_24bits.wav
20, 1:/sine_8000_2ch_16bits_0_75.wav
21, 1:/sine_16000_2ch_16bits_0_75.wav
22, 1:/sine_32000_2ch_16bits_0_75.wav
23, 1:/sine_48000_2ch_16bits_0_75.wav
24, 1:/chrip_48000_2ch_16bits_0_80.wav
25, 1:/play_1ksin_8k_16b.wav
26, 1:/play_1ksin_32k_16b.wav
wav file list complete!

Please open the wav file you want use "wav_open <path>" command.
wav_open 1:/music_48000_2ch_16bits.wav
wav file info:
BMS>>   sample_rate: 48000
        channels: 2
        bits: 16
        size: 3490796
        samples: 872699

Please select lc3 preset use "lc3_preset <name>" command.

BMS>>
BMS>>
BMS>> lc3_preset 48_1_1
48_1_1:
        codec_cfg - sample_rate: 48000, duration: 7500, len: 75
        qos - interval: 7500, framing: 0, phy: 2, sdu: 75, rtn: 4, pd: 40000
BMS>> LC3 encoder setup done!
Creating broadcast source
Creating broadcast source with 1 subgroups with 2 streams
Starting broadcast source
Broadcast source started
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Procedures to run
1 input "help" to show command list
2 input "wav_open <path>" open the wav file.
3 input "lc3_preset <name>" load lc3 preset, then the broadcast will start.
4 input "pause" to stop the broadcast.
5 input "play" to start broadcast.

Other cmds could be used:
1 "lc3_preset_list" used to list all the lc3 preset this demo support.
2 "sync_info" used to get iso_interval/sync_delay, and this cmd should be used after the audio stream start.
3 "config_rtn" used to config the rtn, and this cmd should be used before "lc3_preset".
4 "config_pd" used to config the pd, and this cmd should be used before "lc3_preset".
5 "config_phy" used to config the phy, and this cmd should be used before "lc3_preset".
6 "config_packing" used to config iso packing mode, and this cmd should be used before "lc3_preset".
7 "set_broadcast_code" used to set broadcast code, and this cmd should be used before "lc3_preset".

Note:
1 "exit" command is a shell internal command, only used to exit shell module and could not used to exit demo.
2 the "music_16_2.wav" should be 16/24/32bits 2 channel with sample rate 8000/16000/24000/32000/48000.
