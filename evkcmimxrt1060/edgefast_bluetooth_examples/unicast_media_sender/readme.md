Overview
========
Application demonstrating how to use the unicast media sender feature.

There should be three boards: 1 UMS + 1 UMR(left) + 1 UMR(right).
UMS: unicast stereo audio stream, left channel on first CIS and right channel on another CIS.
UMR: receive one of CIS channel and render it.


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
- evkcmimxrt1060 board
- Personal Computer
- Embedded Artists 2EL M.2 Module (Rev-A1) - direct M2 connection.

Board settings
==============
Before building the example application select Wi-Fi module macro in the app_bluetooth_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
If you want to use Embedded Artists 2EL M.2 Module(Rev-A1), please change the macro to WIFI_IW612_BOARD_MURATA_2EL_M2.

Jumper settings for RT1060-EVKC (enables external 5V supply):
remove  J40 5-6
connect J40 1-2
connect J45 with external power(controlled by SW6)

Murata Solution Board settings
Embedded Artists 2EL module datasheet: https://www.embeddedartists.com/doc/ds/2EL_M2_Datasheet.pdf

The hardware should be reworked according to the hardware rework guide for evkcmimxrt1060 and Murata 1ZM M.2 Adapter in document Hardware Rework Guide for EdgeFast BT PAL.

Note:
To ensure that the LITTLEFS flash region has been cleaned,
all flash sectors need to be erased before downloading example code.
After downloaded binary into qspiflash and boot from qspiflash directly,
please reset the board by pressing SW7 or power off and on the board to run the application.
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
Copyright  2022  NXP

UMS>> 
Unicast Media Sender.
Initializing
Initialized
wav file info:
	sample_rate: 16000
	channels: 2
	bits: 16
	size: 3827552
	samples: 956888
LC3 encoder setup done!
Creating unicast group
Unicast group created
Waiting for connection
Scan & connect left cis sink:
Scanning successfully started
Device found: D0:17:69:EE:69:1B (public) (RSSI -70)
[AD]: 1 data_len 1
[AD]: 3 data_len 2
[AD]: 22 data_len 8
[AD]: 9 data_len 28
[device name]: unicast_media_receiver_right
Device found: D0:17:69:EE:68:9D (public) (RSSI -74)
Device found: D0:17:69:EE:68:9D (public) (RSSI -75)
Device found: 9C:19:C2:3E:D7:9B (public) (RSSI -102)
Device found: D0:17:69:EE:68:9D (public) (RSSI -75)
Device found: D0:17:69:EE:69:1B (public) (RSSI -71)
Device found: D0:17:69:EE:69:1B (public) (RSSI -70)
[AD]: 1 data_len 1
[AD]: 3 data_len 2
[AD]: 22 data_len 8
[AD]: 9 data_len 28
[device name]: unicast_media_receiver_right
Device found: D0:17:69:EE:68:9D (public) (RSSI -75)
Device found: D0:17:69:EE:68:9D (public) (RSSI -75)
Device found: CB:6D:B6:97:27:B7 (random) (RSSI -89)
Device found: CE:1D:86:26:FE:B7 (random) (RSSI -93)
Device found: D5:70:14:34:A1:B4 (random) (RSSI -96)
Device found: 6F:AD:70:37:8D:1E (random) (RSSI -92)
Device found: 70:33:9B:81:F7:C0 (random) (RSSI -96)
Device found: 5C:54:63:FD:86:66 (random) (RSSI -97)
Device found: C9:0F:39:26:C1:CC (random) (RSSI -83)
Device found: CE:1D:86:26:FE:B7 (random) (RSSI -96)
Device found: 6D:A4:6E:CC:F1:18 (random) (RSSI -86)
Device found: 7A:A1:AC:F1:DA:62 (random) (RSSI -86)
Device found: A0:CD:F3:77:E6:15 (public) (RSSI -72)
Device found: 7A:A1:AC:F1:DA:62 (random) (RSSI -87)
Device found: CB:6D:B6:97:27:B7 (random) (RSSI -92)
Device found: 9C:19:C2:3E:D7:9B (public) (RSSI -77)
Device found: 6D:A4:6E:CC:F1:18 (random) (RSSI -92)
Device found: 55:78:0F:26:5E:2B (random) (RSSI -84)
Device found: CE:1D:86:26:FE:B7 (random) (RSSI -93)
Device found: A0:CD:F3:77:E6:15 (public) (RSSI -71)
Device found: C9:0F:39:26:C1:CC (random) (RSSI -92)
Device found: E1:C1:6E:2E:9D:E0 (random) (RSSI -94)
Device found: 62:BE:4B:FA:FE:07 (random) (RSSI -82)
Device found: C9:0F:39:26:C1:CC (random) (RSSI -83)
Device found: CE:1D:86:26:FE:B7 (random) (RSSI -96)
Device found: 7A:A1:AC:F1:DA:62 (random) (RSSI -87)
Device found: 73:44:01:AE:44:62 (random) (RSSI -95)
Device found: CB:6D:B6:97:27:B7 (random) (RSSI -92)
Device found: D0:17:69:EE:69:1B (public) (RSSI -37)
[AD]: 1 data_len 1
[AD]: 3 data_len 2
[AD]: 22 data_len 8
[AD]: 9 data_len 28
[device name]: unicast_media_receiver_right
Device found: 6D:A4:6E:CC:F1:18 (random) (RSSI -94)
Device found: 5C:54:63:FD:86:66 (random) (RSSI -95)
Device found: 63:AF:73:23:88:87 (random) (RSSI -96)
Device found: D0:17:69:EE:68:9D (public) (RSSI -40)
[AD]: 1 data_len 1
[AD]: 3 data_len 2
[AD]: 22 data_len 8
[AD]: 9 data_len 27
[device name]: unicast_media_receiver_left
Audio server found; connecting
MTU exchanged: 23/23
Connected: D0:17:69:EE:68:9D (public)
MTU exchanged: 65/65
Connected
Waiting for connection
Scan & connect right cis sink:
Scanning successfully started
Device found: CE:1D:86:26:FE:B7 (random) (RSSI -94)
Device found: 6D:A4:6E:CC:F1:18 (random) (RSSI -82)
Device found: 9C:19:C2:3E:D7:9B (public) (RSSI -72)
Device found: C9:0F:39:26:C1:CC (random) (RSSI -84)
Device found: E6:DD:1E:0C:A6:83 (random) (RSSI -89)
Device found: 57:1C:5A:5F:71:E3 (random) (RSSI -94)
Device found: F4:DB:3B:C2:D6:3F (random) (RSSI -100)
Device found: CE:1D:86:26:FE:B7 (random) (RSSI -96)
Device found: 6D:A4:6E:CC:F1:18 (random) (RSSI -92)
Device found: 7A:A1:AC:F1:DA:62 (random) (RSSI -87)
Device found: 57:1C:5A:5F:71:E3 (random) (RSSI -92)
Device found: CB:6D:B6:97:27:B7 (random) (RSSI -92)
Device found: 62:BE:4B:FA:FE:07 (random) (RSSI -84)
Device found: 55:78:0F:26:5E:2B (random) (RSSI -75)
Device found: 5C:B7:95:12:4A:D6 (random) (RSSI -92)
Device found: D0:B4:5D:59:C0:D1 (public) (RSSI -73)
Device found: C9:0F:39:26:C1:CC (random) (RSSI -93)
Device found: D0:17:69:EE:69:1B (public) (RSSI -36)
[AD]: 1 data_len 1
[AD]: 3 data_len 2
[AD]: 22 data_len 8
[AD]: 9 data_len 28
[device name]: unicast_media_receiver_right
Audio server found; connecting
MTU exchanged: 23/23
Connected: D0:17:69:EE:69:1B (public)
MTU exchanged: 65/65
Connected
Discover VCS

VCS discover finished
Discover VCS complete.
Discovering sinks
codec_capabilities 2000C4EC dir 0x01
codec 0x06 cid 0x0000 vid 0x0000 count 5
data #0: type 0x01 len 2
f5
data #1: type 0x02 len 1

data #2: type 0x03 len 1

data #3: type 0x04 len 4
280078
data #4: type 0x05 len 1

meta #0: type 0x01 len 2
06
dir 1 loc 1
snk ctx 31 src ctx 0
Sink #0: ep 202D6DB0
Discover sinks complete: err 0
Sinks discovered
Configuring streams
Audio Stream 202EBC64 configured
Configured sink stream[0]
Stream configured
Setting stream QoS
QoS: waiting for 0 streams
Audio Stream 202EBC64 QoS set
Stream QoS Set
Enabling streams
Audio Stream 202EBC64 enabled
Streams enabled
Starting streams
Audio Stream 202EBC64 started
Streams started
Discover VCS

VCS discover finished
Discover VCS complete.
Discovering sinks
codec_capabilities 2000C4EC dir 0x01
codec 0x06 cid 0x0000 vid 0x0000 count 5
data #0: type 0x01 len 2
f5
data #1: type 0x02 len 1

data #2: type 0x03 len 1

data #3: type 0x04 len 4
280078
data #4: type 0x05 len 1

meta #0: type 0x01 len 2
06
dir 1 loc 2
snk ctx 31 src ctx 0
Sink #1: ep 202D72A0
Discover sinks complete: err 0
Sinks discovered
Configuring streams
Audio Stream 202EBC8C configured
Configured sink stream[1]
Stream configured
Setting stream QoS
QoS: waiting for 1 streams
Audio Stream 202EBC8C QoS set
Stream QoS Set
Enabling streams
Audio Stream 202EBC8C enabled
Streams enabled
Starting streams
Audio Stream 202EBC8C started
Streams started
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Procedures to run
1 input "help" to show command list.
2 make sure UMR left and right are initialized and start advertising.
3 the source will start to scan and connect to UMR automatically, then start playing after all UMR connected and configured.
4 input "pause" to stop playing.
5 input "play" to start playing.
6 input "vol_up", "vol_down", "vol_set" to set volume of all sinks.
7 input "vol_mute", "vol_unmute" to set mute of all sinks.

Note:
1 "exit" command is a shell internal command, only used to exit shell module and could not used to exit demo.
2 the "music_16_2.wav" should be 16/24/32bits 2 channel with sample rate 8000/16000/24000/32000/48000.
