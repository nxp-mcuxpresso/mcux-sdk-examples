Overview
========
This example demonstrates the HFP HF basic functionality, the HFP device support be connected to a HFP AG like a mobile phone or a 
board running a HFP AG application. And the HF example support accept/reject/End the incoming call from HFP AG. 



SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- MCUXpresso  11.8.0
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- evkmimxrt1040 board
- Personal Computer
- One of the following modules:
  - AzureWave AW-CM358MA.M2
  - AzureWave AW-CM510MA.M2
  - Embedded Artists 1XK M.2 Module (EAR00385)
  - Embedded Artists 1ZM M.2 Module (EAR00364)


Board settings
==============
Before building the example application select Wi-Fi module macro in the app_bluetooth_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
If you want use the AzureWave WIFI_IW416_BOARD_AW_AM510MA, please change the macro to WIFI_IW416_BOARD_AW_AM510MA.
If you want use the AzureWave WIFI_88W8987_BOARD_AW_CM358MA, please change the macro to WIFI_88W8987_BOARD_AW_CM358MA.
If you want to use Embedded Artists Type 1XK module (EAR00385), please change the macro to WIFI_IW416_BOARD_MURATA_1XK_M2.
If you want to use Embedded Artists Type 1ZM module (EAR00364), please change the macro to WIFI_88W8987_BOARD_MURATA_1ZM_M2.


Jumper settings for RT1040 (enables external 5V supply):
connect J45 with external power
SW6 2-3
J40 1-2


Murata Solution Board settings
Embedded Artists M.2 module resource page: https://www.embeddedartists.com/m2
Embedded Artists 1XK module datasheet: https://www.embeddedartists.com/doc/ds/1XK_M2_Datasheet.pdf
Embedded Artists 1ZM module datasheet: https://www.embeddedartists.com/doc/ds/1ZM_M2_Datasheet.pdf


AzureWave Solution Board settings
The hardware should be reworked according to the hardware rework guide for evkmimxrt1040 with Direct Murata M.2 Module in document Hardware Rework Guide for EdgeFast BT PAL.


Note:
To ensure that the LITTLEFS flash region has been cleaned,
all flash sectors need to be erased before downloading example code.
After downloaded binary into qspiflash and boot from qspiflash directly,
please reset the board by pressing SW7 or power off and on the board to run the application.
Prepare the Demo
================

1.  Open example's project and build it.

2.  Connect a USB cable between the PC host and the OpenSDA USB port on the target board.

3.  Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control

4.  Download the program to the target board.

5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
USB Host stack successfully initialized
Bluetooth initialized
BR/EDR set connectable and discoverable done
Now Start SDP Service and the Service is now discoverable by remote device
>>
the bellow commands have been supported:
"bt": BT related function
 USAGE: bt [dial|aincall|eincall]
    dial          dial out call.
    aincall       accept the incoming call.
    eincall       end an incoming call.
    svr           start voice recognition.
    evr           stop voice recognition.
    clip          enable CLIP notification.
    disclip       disable CLIP notification.
    ccwa          enable call waiting notification.
    disccwa       disable call waiting notification.
    micVolume     Update mic Volume.
    speakerVolume Update Speaker Volume.
    lastdial      call the last dial number.
    voicetag      Get Voice-tag Phone Number (BINP).
    multipcall    multiple call option
    
1) "dial" is used to dial out a call with phone number after device is connected, usage :
   bt dial 114
2) "aincall" is used to accept the incoming call when a call is coming, usage :
   bt aincallis 
3) "eincall" is used to reject the incoming call when a call is coming or end an active call
   bt eincall 
4) "svr" is used to start voice recognition, you can check the voice recognition information in peer device side.
   HFP voice recognition :1
5) "evr" is used to stop voice recognition, you can check the voice recognition information in peer device side.
   HFP voice recognition :0
6) "clip" is used to enable CLIP notification, you can see the incoming call phone number is showing in screen when enable the feature 
   Incoming Call...
   Phone call number: 133xxxxxxxx
7) "disclip" is used to disable CLIP notification, you can't see the incoming call phone number is showing in screen when enable the feature
8) "ccwa" is used to enable enable call waiting notification, you can in waiting call phone number is showing in screen when enable the feature and have multiple call
	>> > CALL WAITING Received Number : 133xxxxxxxx
	> Please use <multipcall> to handle multipe call operation
	 bt multipcall 0. Release all Held Calls and set UUDB tone (Reject new incoming waiting call)
	 bt multipcall 1. Release Active Calls and accept held/waiting call
	 bt multipcall 2. Hold Active Call and accept already held/new waiting call
	 bt multipcall 3. Conference all calls
	 bt multipcall 4. Connect other calls and disconnect self from TW
9) "disccwa" is used to disable call waiting notification, you can in waiting call phone number is not showing in screen when enable the feature and have multiple call
10) "micVolume" is used to set mic volume, the value  is from 1 to 15, usage as:
    bt micVolume 8
11) "speakerVolume" is used to set speaker volume, the value is from 1 to 15, usage as:
    bt speakerVolume 8
12) "lastdial" is used to call the last dial number.
    bt lastdial
13) "voicetag" is used to get Voice-tag Phone Number (BINP), need peer side to set Voice-tag Phone Number (BINP)
    bt lastdial
14) "multipcall"  is used call option, need peer side to set Voice-tag Phone Number (BINP), you can refer to ccwa information to do operation, usage:
    bt multipcall 1

Note:
There is a short noise can be heard at headset at the begin audio streaming when in running HFP Unit and HFP ring tone   
and at the end of each ring tone segment. The codec power on pop noise cannot eliminate.


