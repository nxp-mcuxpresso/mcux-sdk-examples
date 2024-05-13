Overview
========
Application demonstrating how to use the broadcast media sender feature.

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
Broadcast Media Sender.
Bluetooth initialized
wav file info:
	sample_rate: 16000
	channels: 2
	bits: 16
	size: 3827552
	samples: 956888
LC3 encoder setup done!
Codec setup done!
Qos setup done!
Creating broadcast source
Creating broadcast source with 1 subgroups with 2 streams
Starting broadcast source
Broadcast source started
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Procedures to run
1 input "help" to show command list
2 the broadcast will start automatically.
3 input "pause" to stop the broadcast.
4 input "play" to start broadcast.

Note:
1 "exit" command is a shell internal command, only used to exit shell module and could not used to exit demo.
2 the "music_16_2.wav" should be 16/24/32bits 2 channel with sample rate 8000/16000/24000/32000/48000.
