Overview
========
This example demonstrates the PBAP PCE basic functionality, the PCE support be connected to a PSE like a mobile phone or a 
board running a PSE application. 



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
- evkcmimxrt1060 board
- Personal Computer
- One of the following modules:
  - Embedded Artists 1XK M.2 Module (EAR00385) - direct M2 connection.
  - Embedded Artists 1ZM M.2 Module (EAR00364) - direct M2 connection.
  - Embedded Artists 2EL M.2 Module (Rev-A1) - direct M2 connection.

Board settings
==============
Before building the example application select Wi-Fi module macro in the app_bluetooth_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
If you want to use Embedded Artists 1XK M.2 Module(EAR00385), please change the macro to WIFI_IW416_BOARD_MURATA_1XK_M2.
If you want to use Embedded Artists 1ZM M.2 Module(EAR00364), please change the macro to WIFI_88W8987_BOARD_MURATA_1ZM_M2.
If you want to use Embedded Artists 1ZM M.2 Module(Rev-A1), please change the macro to WIFI_IW612_BOARD_MURATA_2EL_M2.

Jumper settings for RT1060-EVKC (enables external 5V supply):
remove  J40 5-6
connect J40 1-2
connect J45 with external power(controlled by SW6)

Murata Solution Board settings
Embedded Artists 1XK module datasheet: https://www.embeddedartists.com/doc/ds/1XK_M2_Datasheet.pdf
Embedded Artists 1ZM module datasheet: https://www.embeddedartists.com/doc/ds/1ZM_M2_Datasheet.pdf
Embedded Artists 2EL module datasheet: https://www.embeddedartists.com/doc/ds/2EL_M2_Datasheet.pdf

The hardware should be reworked according to the hardware rework guide for evkcmimxrt1060 and Murata 1XK M.2 Adapter in document Hardware Rework Guide for EdgeFast BT PAL.

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

3.  Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control

4.  Download the program to the target board.

5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

This demo runs fully automatic. The user needs to place the PSE device that wants to be connected as close as possible to the PCE.
Here's a log of a sample run. PSE is a mobile phone.

Bluetooth PBAP PCE demo start...
Bluetooth initialized
BR/EDR set connectable and discoverable done

Discovery started. Please wait ...
BR/EDR discovery complete
[1]: A0:CD:F3:77:F2:DC, RSSI -72 
[2]: 04:6C:59:78:5E:DC, RSSI -71 NXL53507
[3]: F8:E5:CE:19:75:D3, RSSI -69 Chang
[4]: 24:2E:02:C3:75:45, RSSI -41 test
[5]: CC:EB:5E:10:3E:79, RSSI -70 Redmi K700000
[6]: 30:BB:7D:8F:43:34, RSSI -61 AlisaS
[7]: 94:87:E0:83:99:61, RSSI -70 AnChangMI 8 SE
[8]: 98:43:FA:27:41:CD, RSSI -93 

Connect 4
Connection pending

bt_connected
SDP discovery started

sdp success callback
pbap version is 101
pbap pse supported repositories is 1
supported feature not found, use default feature_config : 3
rfcomm channel found. Connecting ...
Successfully START PBAP PCE entities 
Security changed: 24:2E:02:C3:75:45 level 2
PABP CONNECT SUCCESS

BEGIN:VCARD
VERSION:2.1
FN;CHARSET=UTF-8:descvs
N;CHARSET=UTF-8:descvs
END:VCARD
BEGIN:VCARD
VERSION:2.1
N:;cc;;;
FN:cc
TEL;CELL:154555845
END:VCARD
BEGIN:VCARD
VERSION:2.1
N:;qwe;;;
FN:qwe
X-ANDROID-CUSTOM:vnd.android.cursor.item/nickname;147;1;;;;;;;;;;;;;
TEL;CELL:151865216
TEL;CELL:153464856
EMAIL;HOME:wudhxjsjd@qq.com
ADR;HOME:;;123456789;;;;
NOTE:old
BDAY:1904-05-24
X-AIM:@qq.com
END:VCARD

pbap pse path set success

<?xml version="1.0"?><!DOCTYPE vcard-listing SYSTEM "vcard-listing.dtd"><vCard-listing version="1.0">
<card handle="1.vcf" name="qwe"/><card handle="2.vcf" name="qwe"/><card handle="3.vcf" name="qwe"/>
<card handle="4.vcf" name="1155"/><card handle="5.vcf" name="051295205593"/><card handle="6.vcf" 
name="130"/><card handle="7.vcf" name="051295205593"/><card handle="8.vcf" name="130"/><card handle
="9.vcf" name="11100000000"/><card handle="10.vcf" name="130"/><card handle="11.vcf" name="051295205
593"/><card handle="12.vcf" name="15850182986"/><card handle="13.vcf" name="15850182986"/><ca
rd handle="14.vcf" name="15850182986"/><card handle="15.vcf" name="15850182986"/></vCard-listing>

pbap pse path set success

BEGIN:VCARD
VERSION:2.1
FN:
N:
TEL;X-0:1155
X-IRMC-CALL-DATETIME;DIALED:20220913T110607
END:VCARD

pbap pse path set success

PABP DISCONNECT SUCCESSL : a0
Disconnected (reason 0x13)

Specific implementation steps:

Step 1：Initialize Bluetooth
log: 
Bluetooth PBAP PCE demo start...
Bluetooth initialized
BR/EDR set connectable and discoverable done

Step 2: Device Discovery
After successful Bluetooth initialization, the PCE automatically discovers the device.
log：
Discovery started. Please wait ...
BR/EDR discovery complete
[1]: A0:CD:F3:77:F2:DC, RSSI -72 
[2]: 04:6C:59:78:5E:DC, RSSI -71 NXL53507
[3]: F8:E5:CE:19:75:D3, RSSI -69 Chang
[4]: 24:2E:02:C3:75:45, RSSI -41 test
[5]: CC:EB:5E:10:3E:79, RSSI -70 Redmi K700000
[6]: 30:BB:7D:8F:43:34, RSSI -61 AlisaS
[7]: 94:87:E0:83:99:61, RSSI -70 AnChangMI 8 SE
[8]: 98:43:FA:27:41:CD, RSSI -93 

Step 3: create ACL connection
The PCE finds the device with the strongest RSSI to connect from the device which cod is computer or phone.
log：
Connect 4
Connection pending

Step 4: sdp_find
After ACL connect successfully, PCE perform an SDP operation to find the PSE service class in the connected device.
log:
bt_connected
SDP discovery started

Step 5: create pbap connection 
After sdp callback successfully, PCE will parse to get the peer_feature, GOEP_L2CAP_PSM and rfcomm channel etc.
Prioritizes the establishment of pbap connections based on GOEP_L2CAP_PSM if supported by the PSE.
Otherwise the connection is established based on the rfcomm channel.
log：
sdp success callback
pbap version is 101
pbap pse supported repositories is 1
supported feature not found, use default feature_config : 3
rfcomm channel found. Connecting ...
Successfully START PBAP PCE entities 
Security changed: 24:2E:02:C3:75:45 level 2
PABP CONNECT SUCCESS

Step 6: pull phonebook object. for example : telecom
After pbap connect successfully, pce active pull phonebook object from pse. for example telecom/pb.vcf.
Parse the acquired information and print.
log:
BEGIN:VCARD
VERSION:2.1
FN;CHARSET=UTF-8:descvs
N;CHARSET=UTF-8:descvs
END:VCARD
BEGIN:VCARD
VERSION:2.1
N:;cc;;;
FN:cc
TEL;CELL:154555845
END:VCARD
BEGIN:VCARD
VERSION:2.1
N:;qwe;;;
FN:qwe
X-ANDROID-CUSTOM:vnd.android.cursor.item/nickname;147;1;;;;;;;;;;;;;
TEL;CELL:151865216
TEL;CELL:153464856
EMAIL;HOME:wudhxjsjd@qq.com
ADR;HOME:;;123456789;;;;
NOTE:old
BDAY:1904-05-24
X-AIM:@qq.com
END:VCARD

Step 7: set path to telecom
Need to set the path before getting vcardlisting.
log：
pbap pse path set success.

Step 8： pull vcard listing object
After set path successfully, pce pull vcard listing object from pse. for example : cch.
Parse the acquired information and print.
log：
<?xml version="1.0"?><!DOCTYPE vcard-listing SYSTEM "vcard-listing.dtd"><vCard-listing version="1.0">
<card handle="1.vcf" name="qwe"/><card handle="2.vcf" name="qwe"/><card handle="3.vcf" name="qwe"/>
<card handle="4.vcf" name="1155"/><card handle="5.vcf" name="051295205593"/><card handle="6.vcf" 
name="130"/><card handle="7.vcf" name="051295205593"/><card handle="8.vcf" name="130"/><card handle
="9.vcf" name="11100000000"/><card handle="10.vcf" name="130"/><card handle="11.vcf" name="051295205
593"/><card handle="12.vcf" name="15850182986"/><card handle="13.vcf" name="15850182986"/><ca
rd handle="14.vcf" name="15850182986"/><card handle="15.vcf" name="15850182986"/></vCard-listing>

Step 9： set path to cch
Need to set the path before getting vcardentry.
log:
pbap pse path set success.

Step 10: pull vcard entry object.
After set path successfully, pce pull vcard entry object from pse. for example : 4.vcf.
Parse the acquired information and print.
log:
BEGIN:VCARD
VERSION:2.1
FN:
N:
TEL;X-0:1155
X-IRMC-CALL-DATETIME;DIALED:20220913T110607
END:VCARD

Step 11: set path to root
log：
pbap pse path set success.

step 12: pbap disconnect
log：
PABP DISCONNECT SUCCESSL : a0

Note:
1: If you don't want to display the acquired information, set the macro CONFIG_BT_PRINTF_VALUE to 0 in app_pbap_pce.c.

2: When connecting to PSE which is a moblie phone, please pay attention to the phone alert messages. In particular, please turn on the permission to allow "Shared Phonebook".
   If PSE is a apple iphone, after connecting for the first time, need to actively turn on the "Share Contacts Permission" on phone and then re-run the demo.

3: PCE will only select the device with the strongest RSSI among those with COD of phone or computer for connection.
   Please place the PSE as close as possible to the PCE.
   You can lift this restriction by blocking the relevant code in the app_discover.c.
