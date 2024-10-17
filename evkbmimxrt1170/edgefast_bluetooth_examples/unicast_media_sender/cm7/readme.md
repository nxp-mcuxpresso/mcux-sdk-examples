Overview
========
Application demonstrating how to use the unicast media sender feature.

There should be three boards: 1 UMS + 1 UMR(left) + 1 UMR(right).
UMS: unicast stereo audio stream, left channel on first CIS and right channel on another CIS.
UMR: receive one of CIS channel and render it.

This demo integrated a2dp sink function for RT1170-EVKB, so that Classic BT and LE Audio can work at the same time.
After enable the macro "CONFIG_BT_A2DP" and "CONFIG_BT_A2DP_SINK", this demo will run as a a2dp_bridge.
The audio stream received by a2dp sink will be streamed to UMS and send out via LE Audio.
There should be four boards for a2dp_bridge: 1 a2dp_source + 1 a2dp_bridge(UMS) + 1 UMR(left) + 1 UMR(right).


SDK version
===========
- Version: 2.16.100

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

Note: 4. and 5. are not required in bridge mode.

Running the demo
================
The log below shows the output of the example in the terminal window.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Copyright  2024  NXP

UMS>>
Unicast Media Sender.
Initializing
Initialized

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

UMS>>
UMS>>
UMS>> wav_open 1:/music_16_2.wav
wav file info:
UMS>>   sample_rate: 16000
        channels: 2
        bits: 16
        size: 3827552
        samples: 956888

Please select lc3 preset use "lc3_preset <name>" command.

UMS>>
UMS>>
UMS>> lc3_preset 16_2_1
16_2_1:
        codec_cfg - sample_rate: 16000, duration: 10000, len: 40
        qos - interval: 10000, framing: 0, phy: 2, sdu: 40, rtn: 2, pd: 40000
UMS>> LC3 encoder setup done!
Creating unicast group
Unicast group created
Please scan and connect the devices you want!

UMS>>
UMS>>
UMS>> scan
Scanning successfully started
UMS>>
UMS>>
UMS>> [0]: CC:F8:26:ED:E7:82 (public), rssi -55, Galaxy Buds2 Pro
[1]: CC:F8:26:F6:87:8C (public), rssi -52, Galaxy Buds2 Pro
connect 0
UMS>> device selected!
Connecting
Connect first device
MTU exchanged: 23/23
LE Connected: CC:F8:26:ED:E7:82 (public)
MTU exchanged: 196/196
Connected
CSIP discover
CSIP conn 202DC9F0 discovered set count 1
set 1/1 info:
        sirk: cc f8 26 f6 87 8c cc f8 26 ed e7 82 cc f8 26 f6
        set_size: 2
        rank: 2
        lockable: 1
CSIP discovered
Scan another member
member: CC:F8:26:F6:87:8C (public), rssi -48, Galaxy Buds2 Pro
Member discovered
Connecting
Connect second device
MTU exchanged: 23/23
LE Connected: CC:F8:26:F6:87:8C (public)
MTU exchanged: 196/196
Connected
CSIP discover
CSIP conn 202DCAEC discovered set count 1
set 1/1 info:
        sirk: cc f8 26 f6 87 8c cc f8 26 ed e7 82 cc f8 26 f6
        set_size: 2
        rank: 1
        lockable: 1
CSIP discovered
Discover VCS

VCS discover finished
Discover VCS complete.
Discovering sinks
codec_cap 2000A19C dir 0x01
codec id 0x06 cid 0x0000 vid 0x0000 count 19
data: type 0x01 value_len 2
b400
data: type 0x02 value_len 1
03
data: type 0x03 value_len 1
01
data: type 0x04 value_len 4
1a009b00
data: type 0x05 value_len 1
01
meta: type 0x01 value_len 2
0700
dir 1 loc 401
snk ctx 4095 src ctx 623
Sink #0: ep 202D9538
Sink #0: ep 202D9600
Discover sinks complete: err 0
Sinks discovered
Configuring streams
Audio Stream 202F26F4 configured
Configured sink stream[0]
Stream configured
Setting stream QoS
QoS: waiting for 0 streams
Audio Stream 202F26F4 QoS set
Stream QoS Set
Enabling streams
Audio Stream 202F26F4 enabled
Streams enabled
Starting streams
Audio Stream 202F26F4 started
Streams started
Discover VCS

VCS discover finished
Discover VCS complete.
Discovering sinks
codec_cap 2000A19C dir 0x01
codec id 0x06 cid 0x0000 vid 0x0000 count 19
data: type 0x01 value_len 2
b400
data: type 0x02 value_len 1
03
data: type 0x03 value_len 1
01
data: type 0x04 value_len 4
1a009b00
data: type 0x05 value_len 1
01
meta: type 0x01 value_len 2
0700
dir 1 loc 802
snk ctx 4095 src ctx 623
Sink #1: ep 202D99B0
Sink #1: ep 202D9A78
Discover sinks complete: err 0
Sinks discovered
Configuring streams
Audio Stream 202F2718 configured
Configured sink stream[1]
Stream configured
Setting stream QoS
QoS: waiting for 1 streams
Audio Stream 202F2718 QoS set
Stream QoS Set
Enabling streams
snk ctx 4091 src ctx 623
Audio Stream 202F2718 enabled
Streams enabled
Starting streams
Audio Stream 202F2718 started
Streams started
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Procedures to run
1 input "help" to show command list.
2 make sure UMR left and right are initialized and start advertising.
3 input "wav_open <path>" to open one wav file.
4 input "lc3_preset <name>" to load one lc3 preset.
5 input "scan" to start scan all sink devices, then use "connect" to connect one of the set member.
Another set member will be connected automatically.
6 the audio will start playing after all config done.
7 input "pause" to stop playing.
8 input "play" to start playing.
9 input "vol_up", "vol_down", "vol_set" to set volume of all sinks.
10 input "vol_mute", "vol_unmute" to set mute of all sinks.

Note:
1 "exit" command is a shell internal command, only used to exit shell module and could not used to exit demo.
2 the "music_16_2.wav" should be 16/24/32bits 2 channel with sample rate 8000/16000/24000/32000/48000.

Other cmds could be used:
1 "lc3_preset_list" used to list all the lc3 preset this demo support.
2 "sync_info" used to get iso_interval/sync_delay, and this cmd should be used after the audio stream start.
3 "config_rtn" used to config the rtn, and this cmd should be used before "lc3_preset".
4 "config_pd" used to config the pd, and this cmd should be used before "lc3_preset".
5 "config_phy" used to config the phy, and this cmd should be used before "lc3_preset".
6 "config_packing" used to config iso packing mode, and this cmd should be used before "lc3_preset".

Running the bridge demo
The log below shows the output of the example in the terminal window.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Bluetooth A2dp Bridge demo start...
Bluetooth initialized
BR/EDR set connectable and discoverable done
BR Connected
BR Security changed: D0:17:69:EE:69:71 level 2
a2dp connected success
a2dp configure sample rate 48000Hz, ch 2
a2dp start playing
Switch role for D0:17:69:EE:69:71, err 0

Copyright  2024  NXP

UMS>>
Unicast Media Sender.
Initializing
Initialized

Please select lc3 preset use "lc3_preset <name>" command.
BR Security changed: D0:17:69:EE:69:71 level 2

UMS>>
UMS>>
UMS>> lc3_preset 48_2_1
48_2_1:
UMS>>   codec_cfg - sample_rate: 48000, duration: 10000, len: 100
        qos - interval: 10000, framing: 0, phy: 2, sdu: 100, rtn: 5, pd: 40000
LC3 encoder setup done!
Creating unicast group
Unicast group created
Please scan and connect the devices you want!

UMS>>
UMS>> scan
Scanning successfully started
UMS>> [0]: CC:F8:26:F6:87:8C (public), rssi -61, Galaxy Buds2 Pro

UMS>>
UMS>>
UMS>> connect 0[1]: CC:F8:26:ED:E7:82 (public), rssi -70, Galaxy Buds2 Pro

UMS>> device selected!
Connecting
Connect first device
MTU exchanged: 23/23
LE Connected: CC:F8:26:F6:87:8C (public)
MTU exchanged: 196/196
Connected
CSIP discover
CSIP conn 202DD7B8 discovered set count 1
set 1/1 info:
        sirk: cc f8 26 f6 87 8c cc f8 26 ed e7 82 cc f8 26 f6
        set_size: 2
        rank: 1
        lockable: 1
CSIP discovered
Scan another member
member: CC:F8:26:ED:E7:82 (public), rssi -50, Galaxy Buds2 Pro
Member discovered
Connecting
Connect second device
MTU exchanged: 23/23
LE Connected: CC:F8:26:ED:E7:82 (public)
MTU exchanged: 196/196
Connected
CSIP discover
CSIP conn 202DD8B4 discovered set count 1
set 1/1 info:
        sirk: cc f8 26 f6 87 8c cc f8 26 ed e7 82 cc f8 26 f6
        set_size: 2
        rank: 2
        lockable: 1
CSIP discovered
Discover VCS

VCS discover finished
Discover VCS complete.
Discovering sinks
codec_cap 2001A21C dir 0x01
codec id 0x06 cid 0x0000 vid 0x0000 count 19
data: type 0x01 value_len 2
b400
data: type 0x02 value_len 1
03
data: type 0x03 value_len 1
01
data: type 0x04 value_len 4
1a009b00
data: type 0x05 value_len 1
01
meta: type 0x01 value_len 2
0700
dir 1 loc 802
snk ctx 4095 src ctx 623
Sink #0: ep 202D9D10
Sink #0: ep 202D9DD8
Discover sinks complete: err 0
Sinks discovered
Configuring streams
Audio Stream 202F8964 configured
Configured sink stream[0]
Stream configured
Setting stream QoS
QoS: waiting for 0 streams
Audio Stream 202F8964 QoS set
Stream QoS Set
Enabling streams
snk ctx 4091 src ctx 623
Audio Stream 202F8964 enabled
Streams enabled
Starting streams
Audio Stream 202F8964 started
Streams started
Discover VCS

VCS discover finished
Discover VCS complete.
Discovering sinks
codec_cap 2001A21C dir 0x01
codec id 0x06 cid 0x0000 vid 0x0000 count 19
data: type 0x01 value_len 2
b400
data: type 0x02 value_len 1
03
data: type 0x03 value_len 1
01
data: type 0x04 value_len 4
1a009b00
data: type 0x05 value_len 1
01
meta: type 0x01 value_len 2
0700
dir 1 loc 401
snk ctx 4095 src ctx 623
Sink #1: ep 202DA188
Sink #1: ep 202DA250
Discover sinks complete: err 0
Sinks discovered
Configuring streams
Audio Stream 202F8988 configured
Configured sink stream[1]
Stream configured
Setting stream QoS
QoS: waiting for 1 streams
Audio Stream 202F8988 QoS set
Stream QoS Set
Enabling streams
snk ctx 4091 src ctx 623
Audio Stream 202F8988 enabled
Streams enabled
Starting streams
Audio Stream 202F8988 started
Streams started
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Procedures to run a2dp_bridge
1 make sure UMR left and right are initialized and start advertising.
2 wait a2dp_source connect and config the a2dp stream.
3 input "lc3_preset <name>" to load one lc3 preset.
4 input "scan" to start scan all sink devices, then use "connect" to connect one of the set member.
Another set member will be connected automatically.
5 the audio will start playing after all config done.
6 input "pause" to stop playing.
7 input "play" to start playing.
8 input "vol_up", "vol_down", "vol_set" to set volume of all sinks.
9 input "vol_mute", "vol_unmute" to set mute of all sinks.

Other cmds could be used:
1 "lc3_preset_list" used to list all the lc3 preset this demo support.
2 "sync_info" used to get iso_interval/sync_delay, and this cmd should be used after the audio stream start.
3 "config_rtn" used to config the rtn, and this cmd should be used before "lc3_preset".
4 "config_pd" used to config the pd, and this cmd should be used before "lc3_preset".
5 "config_phy" used to config the phy, and this cmd should be used before "lc3_preset".
6 "config_packing" used to config iso packing mode, and this cmd should be used before "lc3_preset".

Note: the a2dp_bridge could only work with a2dp_source right now.
