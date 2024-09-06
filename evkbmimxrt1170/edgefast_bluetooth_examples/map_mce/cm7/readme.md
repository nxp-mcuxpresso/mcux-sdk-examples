Overview
========
This example demonstrates the MAP MCE basic functionality, the MCE device support be connected to a MAP MSE like a mobile phone or a 
board running a MAP MSE application. And the MCE example support browsing/uploading message on/to the MSE and receiving events from MSE. 



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

This demo runs fully automatically. The user needs to place the MSE device to be connected as close to the MCE as possible.
The MCE will only select the device with the strongest RSSI among those with the class of device of phone or computer for connection.
Here's a log of a sample run. MSE is a mobile phone.

Bluetooth MAP MCE demo start...
Bluetooth initialized
BR/EDR set connectable and discoverable done
Discovery started. Please wait ...
BR/EDR discovery complete
[1]: CC:EB:5E:10:3E:79, RSSI -93 test
Connect 1
Connection pending
SDP discovery started
Connected
sdp success callback
REFCOMM channel number 26
L2CAP PSM  0x1029
MAP version 0x0104
MAP supported features 0x000603FF
MAS instance ID 0
Supported message type 0x00
Service name SMS/MMS
Message Access Server found. Connecting ...
Security changed: CC:EB:5E:10:3E:79 level 2
MCE MAS connection
MAX Packet Length - 509
[1]: GET_FOLDER_LISTING_ROOT
MAP Get Folder Listing
MAP Get Folder Listing CNF - 0xA0
<?xml version='1.0' encoding='utf-8' standalone='yes' ?>
<folder-listing version="1.0">
    <folder name="telecom" />
</folder-listing>

[2]: GET_FOLDER_LISTING_ROOT Complete
[3]: SET_FOLDER_TELECOM
MAP Set Folder
Name - telecom
MAP Set Folder CNF - 0xA0
[4]: SET_FOLDER_TELECOM Complete
[5]: SET_FOLDER_MSG
MAP Set Folder
Name - msg
MAP Set Folder CNF - 0xA0
[6]: SET_FOLDER_MSG Complete
[7]: SET_FOLDER_INBOX
MAP Set Folder
Name - inbox
MAP Set Folder CNF - 0xA0
[8]: SET_FOLDER_INBOX Complete
[9]: UPDATE_INBOX
MAP Update Inbox
MAP Update Inbox CNF - 0xD1
[10]: UPDATE_INBOX Complete
[11]: GET_MSG_LISTING
MAP Get MSG Listing
MAX List Count - 10
SRMP Wait Count - 2
MAP Get MSG Listing CNF - 0x90
New Message - 1
Listing Size - 10
MSE Time - 20240716T155258+0800
<?xml version='1.0' encoding='utf-8' standalone='yes' ?>
<MAP-msg-listing version="1.0">
    <msg handle="0400000000001577" subject="Bluetooth MAP Test Bluetooth MAP Test Bluetooth MAP Test Bluetooth MAP Test Bluetooth MAP Test Bluetooth MAP Test Bluetooth MAP Test Bluetooth MAP Test Bluetooth MAP Test Bluetooth MAP Test Bluetooth MAP Test Bluetooth MAP Test Bluetooth MAP Test Bluetooth" datetime="20240716T154618" sender_name="" sender_addressing="+XXXXXXXXXXXXX"
MAP Get MSG Listing CNF - 0xA0
 recipient_name="" recipient_addressing="+XXXXXXXXXXXXX" type="SMS_GSM" size="588" text="yes" reception_status="complete" attachment_size="0" priority="no" read="no" sent="no" protected="no" />
</MAP-msg-listing>

[12]: GET_MSG_LISTING Complete
[13]: GET_MSG
MAP Get MSG
Name - 0400000000001577
Attachment - 0
Charset - 0
SRMP Wait Count - 0
MAP Get MSG CNF - 0x90
BEGIN:BMSG
VERSION:1.0
STATUS:UNREAD
TYPE:SMS_GSM
FOLDER:telecom/msg/inbox
BEGIN:VCARD
VERSION:3.0
FN:
N:
TEL:+XXXXXXXXXXXXX
END:VCARD
BEGIN:BENV
BEGIN:BBODY
ENCODING:G-7BIT
LENGTH:1334
BEGIN:MSG
00440d91XXXXXXXXXXXXfX000042706151648123a0050003a8040184ec7a99fe7ed3d1a066100aa296e77490905d2fd3df6f3a1ad40c4241d4f29c0e12b2eb65fafb4d47839a4128885a9ed3414276bd4c7fbfe9685033080551cb733a48c8ae97e9ef371d0d6a06a1206a794e0709d9f532fdfda6a341cd2014442dcfe92021bb5ea6bfdf7434a8198482a8e5391d2
MAP Get MSG CNF - 0xA0
dfda6a341cd2014442dcfe9
END:MSG
END:BBODY
END:BENV
END:BMSG

[14]: GET_MSG Complete
[15]: SET_MSG_STATUS
MAP Set MSG Status
Name - 0400000000001577
Status Indicator - 0
Status Value - 0
MAP Set MSG Status CNF - 0xA0
[16]: SET_MSG_STATUS Complete
[17] ~ [18] Skip, BT_MAP_CONVO_LISTING is not supported
[19]: GET_MAS_INST_INFO
MAP Get MAS Instance Info
MAS Instance ID - 0
SRMP Wait Count - 0
MAP Get MAS Instance Info CNF - 0xA0
SMS/MMS
[20]: GET_MAS_INST_INFO Complete
[21]: SET_NTF_FILTER
MAP Set Notification Filter
Notification Filter Mask - 0
MAP Set NTF Filter CNF - 0xA0
[22]: SET_NTF_FILTER Complete
[23]: SET_NTF_REG_ON
MAP Set Notification Registration
Notification Status - 1
MAP Set Notification Registration CNF - 0xA0
MCE MNS connection
MAX Packet Length - 1790
[24]: SET_NTF_REG_ON Complete
[25]: SET_NTF_REG_OFF
MAP Set Notification Registration
Notification Status - 0
MAP Set Notification Registration CNF - 0xA0
MCE MNS disconnection - 0xA0
[26]: SET_NTF_REG_OFF Complete
[27] ~ [30] Skip, BT_MAP_OWNER_STATUS is not supported
[31]: SET_FOLDER_PARENT
MAP Set Folder
Name - ../
MAP Set Folder CNF - 0xA0
[32]: SET_FOLDER_PARENT Complete
[33]: SET_FOLDER_OUTBOX
MAP Set Folder
Name - outbox
MAP Set Folder CNF - 0xA0
[34]: SET_FOLDER_OUTBOX Complete
[35]: PUSH_MSG
MAP Push MSG
Charset - 0
MAP Push MSG CNF - 0x90
MAP Push MSG CNF - 0x90
MAP Push MSG CNF - 0x90
MAP Push MSG CNF - 0xA0
Name - 278DE6BF5BF6491
[36]: PUSH_MSG Complete
[37]: MCE_MAS_DISCONNECT
MAP MCE MAS Disconnect
MCE MAS disconnection - 0xA0
Disconnected (reason 0x13)

Specific implementation steps:

1) Initialize Bluetooth
log: 
Bluetooth MAP MCE demo start...
Bluetooth initialized
BR/EDR set connectable and discoverable done

2) Device discovery
log:
Discovery started. Please wait ...
BR/EDR discovery complete
[1]: CC:EB:5E:10:3E:79, RSSI -93 test

3) Create ACL connection
The MCE finds the device with the strongest RSSI to connect from the devices that the class of device is computer or phone.
log：
Connect 1
Connection pending

4) SDP discovery
After the ACL connection is established, the MCE performs an SDP discovery.
log:
SDP discovery started
Connected

5) Create MAP MAS connection
After the SDP discovery, the MCE will parse the SDP records to obtain the GoepL2capPsm, RFCOMM channel number and MapSupportedFeatures etc.
Prioritize the establishment of the MAP MAS connection based on GoepL2capPsm if supported by the MSE.
Otherwise the MAP MAS connection is established based on the RFCOMM channel number.
After that, the MCE will automatically send requests to the MSE according to the MapSupportedFeatures.
log:
sdp success callback
REFCOMM channel number 26
L2CAP PSM  0x1029
MAP version 0x0104
MAP supported features 0x000603FF
MAS instance ID 0
Supported message type 0x00
Service name SMS/MMS
Message Access Server found. Connecting ...
Security changed: CC:EB:5E:10:3E:79 level 2
MCE MAS connection
MAX Packet Length - 509

6) Get folder listing
After MAP MAS connection is established, the MCE will get folder listing in the root directory, and then parse and print the acquired information.
log:
[1]: GET_FOLDER_LISTING_ROOT
MAP Get Folder Listing
MAP Get Folder Listing CNF - 0xA0
<?xml version='1.0' encoding='utf-8' standalone='yes' ?>
<folder-listing version="1.0">
    <folder name="telecom" />
</folder-listing>

[2]: GET_FOLDER_LISTING_ROOT Complete

7) Set path to telecom/msg/inbox
If setting path successfully, the MCE will contiune the next steps or stop here.
0xA0 indicates that setting path is success.
log:
[3]: SET_FOLDER_TELECOM
MAP Set Folder
Name - telecom
MAP Set Folder CNF - 0xA0
[4]: SET_FOLDER_TELECOM Complete
[5]: SET_FOLDER_MSG
MAP Set Folder
Name - msg
MAP Set Folder CNF - 0xA0
[6]: SET_FOLDER_MSG Complete
[7]: SET_FOLDER_INBOX
MAP Set Folder
Name - inbox
MAP Set Folder CNF - 0xA0
[8]: SET_FOLDER_INBOX Complete

8) Send UpdateInbox request
After setting path to telecom/msg/inbox successfully, the MCE will send UpdateInbox request and then print the result.
0xD1 indicates that the MSE doesn't implement this fucntion.
log:
[9]: UPDATE_INBOX
MAP Update Inbox
MAP Update Inbox CNF - 0xD1
[10]: UPDATE_INBOX Complete

9) Get message listing
After updating inbox, the MCE will get message listing in the inbox.
When receiving response, parse and print the acquired infomation.
log:
[11]: GET_MSG_LISTING
MAP Get MSG Listing
MAX List Count - 10
SRMP Wait Count - 2
MAP Get MSG Listing CNF - 0x90
New Message - 1
Listing Size - 10
MSE Time - 20240716T155258+0800
<?xml version='1.0' encoding='utf-8' standalone='yes' ?>
<MAP-msg-listing version="1.0">
    <msg handle="0400000000001577" subject="Bluetooth MAP Test Bluetooth MAP Test Bluetooth MAP Test Bluetooth MAP Test Bluetooth MAP Test Bluetooth MAP Test Bluetooth MAP Test Bluetooth MAP Test Bluetooth MAP Test Bluetooth MAP Test Bluetooth MAP Test Bluetooth MAP Test Bluetooth MAP Test Bluetooth" datetime="20240716T154618" sender_name="" sender_addressing="+XXXXXXXXXXXXX"
MAP Get MSG Listing CNF - 0xA0
 recipient_name="" recipient_addressing="+XXXXXXXXXXXXX" type="SMS_GSM" size="588" text="yes" reception_status="complete" attachment_size="0" priority="no" read="no" sent="no" protected="no" />
</MAP-msg-listing>

[12]: GET_MSG_LISTING Complete

10) Get message
After getting message listing successfully, the MCE will select the first message handle in the message listing to get
its content and parse and print the acquired message. If the MCE doesn't find the message handle, it will skip [13] ~ [16].
log:
[13]: GET_MSG
MAP Get MSG
Name - 0400000000001577
Attachment - 0
Charset - 0
SRMP Wait Count - 0
MAP Get MSG CNF - 0x90
BEGIN:BMSG
VERSION:1.0
STATUS:UNREAD
TYPE:SMS_GSM
FOLDER:telecom/msg/inbox
BEGIN:VCARD
VERSION:3.0
FN:
N:
TEL:+XXXXXXXXXXXXX
END:VCARD
BEGIN:BENV
BEGIN:BBODY
ENCODING:G-7BIT
LENGTH:1334
BEGIN:MSG
00440d91688120218956f1000042706151648123a0050003a8040184ec7a99fe7ed3d1a066100aa296e77490905d2fd3df6f3a1ad40c4241d4f29c0e12b2eb65fafb4d47839a4128885a9ed3414276bd4c7fbfe9685033080551cb733a48c8ae97e9ef371d0d6a06a1206a794e0709d9f532fdfda6a341cd2014442dcfe92021bb5ea6bfdf7434a8198482a8e5391d2
MAP Get MSG CNF - 0xA0
dfda6a341cd2014442dcfe9
END:MSG
END:BBODY
END:BENV
END:BMSG

[14]: GET_MSG Complete.

11) Set message status
After getting message, the MCE will set message status to "unread".
log:
[15]: SET_MSG_STATUS
MAP Set MSG Status
Name - 0400000000001577
Status Indicator - 0
Status Value - 0
MAP Set MSG Status CNF - 0xA0
[16]: SET_MSG_STATUS Complete

12) Get conversation listing
If the Conversation Listing feature is supported by the MSE, the MCE will send GetConversationListing request
and parse and print the acquired infomation. Otherwise, skip [17] ~ [18].
log:
[17] ~ [18] Skip, BT_MAP_CONVO_LISTING is not supported

13) Get MAS instance infomation
If the 'Instance Information Feature' bit in the MapSupportedFeatures of the MSE is set, the MCE will send GetMASInstanceInformation request
and parse and print the acquired infomation. Otherwise, skip [19] ~ [20].
log:
[19]: GET_MAS_INST_INFO
MAP Get MAS Instance Info
MAS Instance ID - 0
SRMP Wait Count - 0
MAP Get MAS Instance Info CNF - 0xA0
SMS/MMS
[20]: GET_MAS_INST_INFO Complete

14) Set notification filter
If the 'Notification Filtering' bit in the MapSupportedFeatures of the MSE is set, the MCE will send SetNotificationFilter(NotificationFilterMask = 0) request
and print the result.
log:
[21]: SET_NTF_FILTER
MAP Set Notification Filter
Notification Filter Mask - 0
MAP Set NTF Filter CNF - 0xA0
[22]: SET_NTF_FILTER Complete

15) Set notification registration
If the 'Notification Registration Feature' bit in the MapSupportedFeatures of the MSE is set, the MCE will send SetNotificationRegistration(ON) request
and print the result. After the MSE receives the SetNotificationRegistration(ON) request, it will initiate an MNS connection
to the MCE. When the MNS connection is created successfully, the MCE will print the 'MCE MNS connection'.
After the MNS connection is established, the MCE will send SetNotificationRegistration(OFF) request and print the result.
After the MSE receives the SetNotificationRegistration(OFF) request, it will disconnect the MNS connection.
When the MNS connection is disconnected, the MCE will print the 'MCE MNS disconnection'.
[23]: SET_NTF_REG_ON
MAP Set Notification Registration
Notification Status - 1
MAP Set Notification Registration CNF - 0xA0
MCE MNS connection
MAX Packet Length - 1790
[24]: SET_NTF_REG_ON Complete
[25]: SET_NTF_REG_OFF
MAP Set Notification Registration
Notification Status - 0
MAP Set Notification Registration CNF - 0xA0
MCE MNS disconnection - 0xA0
[26]: SET_NTF_REG_OFF Complete

16) Get and set owner status.
If the 'Owner status' bit in the MapSupportedFeatures of the MSE is set, the MCE will send GetOwnerStatus request,
and then parse and print the acquired infomation. Otherwise, skip [27] ~ [30].
After getting owner status, the MCE will send SetOwnerStatus and print the result.
log:
[27] ~ [30] Skip, BT_MAP_OWNER_STATUS is not supported

17) Set path to ../outbox
If the 'Uploading Feature' bit in the MapSupportedFeatures of the MSE is set, the MCE will go to outbox directory and then send a message to outbox.
log:
[31]: SET_FOLDER_PARENT
MAP Set Folder
Name - ../
MAP Set Folder CNF - 0xA0
[32]: SET_FOLDER_PARENT Complete
[33]: SET_FOLDER_OUTBOX
MAP Set Folder
Name - outbox
MAP Set Folder CNF - 0xA0
[34]: SET_FOLDER_OUTBOX Complete

18) Push message
After setting path to ../outbox, the MCE will push the message and print the result. The pushed message is a constant message for the test.
0x90 and 0xA0 indicates the response code is Continue and Success respectively. After sending the message completely,
the MCE will print the mesage handle recived from the MSE.
log:
[35]: PUSH_MSG
MAP Push MSG
Charset - 0
MAP Push MSG CNF - 0x90
MAP Push MSG CNF - 0x90
MAP Push MSG CNF - 0x90
MAP Push MSG CNF - 0xA0
Name - 278DE6BF5BF6491

19) Send MAP MAS disconnect
All above operations are complete, the MCE will send MAS disconnection request and print the result.
log:
[37]: MCE_MAS_DISCONNECT
MAP MCE MAS Disconnect
MCE MAS disconnection - 0xA0

Note:
   1. When connecting to the MSE which is a moblie phone, please pay attention to the phone alert messages.
      In particular, please turn on the permission to allow access to messages.
      If the MSE is an iPhone, after connecting for the first time, the user need to actively turn on the "Show Notifications" on the iPhone and then re-run the demo.
   2. The MCE will only select the device with the strongest RSSI among those with the class of device of phone or computer for connection.
      Please place the MSE as close as possible to the PCE. You can lift this restriction by blocking the relevant code in the app_discover.c.
   3. Only 1 MAP MAS and MNS connection is supported.
