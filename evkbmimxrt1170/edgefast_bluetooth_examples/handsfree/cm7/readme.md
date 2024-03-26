Overview
========
This example demonstrates the HFP HF basic functionality, the HFP device support be connected to a HFP AG like a mobile phone or a 
board running a HFP AG application. And the HF example support accept/reject/End the incoming call from HFP AG. 



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
- Embedded Artists 1XK M.2 Module (EAR00385) - direct M2 connection.
- Embedded Artists 1ZM M.2 Module (EAR00364) - direct M2 connection.
- Embedded Artists 2EL M.2 Module (Rev-A1) - direct M2 connection.

Jumper settings for RT1170-EVKB (enables external 5V supply):
remove  J38 5-6
connect J38 1-2
connect J43 with external power(controlled by SW5)

Murata Solution Board settings
Embedded Artists M.2 module resource page: https://www.embeddedartists.com/m2
Embedded Artists 1XK module datasheet: https://www.embeddedartists.com/doc/ds/1XK_M2_Datasheet.pdf
Embedded Artists 1ZM module datasheet: https://www.embeddedartists.com/doc/ds/1ZM_M2_Datasheet.pdf
Embedded Artists 2EL module datasheet: https://www.embeddedartists.com/doc/ds/2EL_M2_Datasheet.pdf

The hardware should be reworked according to the Hardware Rework Guide for MIMXRT1170-EVKB and Murata 1XK M.2 Adapter in document Hardware Rework Guide for EdgeFast BT PAL.
The hardware should be reworked according to the Hardware Rework Guide for MIMXRT1170-EVKB and Murata 1ZM M.2 Adapter in document Hardware Rework Guide for EdgeFast BT PAL.
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


