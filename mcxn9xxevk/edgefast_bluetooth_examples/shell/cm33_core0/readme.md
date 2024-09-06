Overview
========
Application demonstrating the shell mode of the simplified Adapter APIs.


SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- MCNX947-EVK board
- Personal Computer
- One of the following modules:
  - Embedded Artists 1XK M.2 Module (EAR00385) - direct M2 connection.
  - Embedded Artists 1ZM M.2 Module (EAR00364) - direct M2 connection.

Board settings
==============
Before building the example application select Wi-Fi module macro in the app_bluetooth_config.h. (see #define WIFI_<SoC Name>_BOARD_<Module Name>).
If you want to use Embedded Artists Type 1XK module(EAR00385 M2 only), please change the macro to WIFI_IW416_BOARD_MURATA_1XK_M2.
If you want to use Embedded Artists Type 1ZM module(EAR00364 M2 only), please change the macro to WIFI_88W8987_BOARD_MURATA_1ZM_M2.

Hardware rework guide:
The hardware should be reworked according to the Hardware Rework Guide for MCXN947-EVK with Direct Murata M.2 Module in document Hardware Rework Guide for EdgeFast BT PAL.

Murata Solution Board settings
Murata uSD-M.2 adapter resource page: https://www.murata.com/en-us/products/connectivitymodule/wi-fi-bluetooth/overview/lineup/usd-m2-adapter
Murata uSD-M.2 adapter datasheet: https://www.murata.com/-/media/webrenewal/products/connectivitymodule/asset/pub/rfm/data/usd-m2_revb1.ashx
Embedded Artists M.2 module resource page: https://www.embeddedartists.com/m2
Embedded Artists 1XK module datasheet: https://www.embeddedartists.com/doc/ds/1XK_M2_Datasheet.pdf
Embedded Artists 1ZM module datasheet: https://www.embeddedartists.com/doc/ds/1ZM_M2_Datasheet.pdf

Note:
For Peripheral DMA to be used, the application core must first gain TRDC access from edgelock FW, so this demo
must be runnning with edgelock FW alive.
Whole memory must be erased before this demo is flashed.
After downloaded binary into Quad SPI Flash and boot from Quad SPI Flash directly,
please reset the board by pressing SW3 or power off and on the board to run the application.

If you want to get HCI log , please define macro CONGIF_BT_SNOOP to 1 in app_bluetooth_config.h , then connect JUMP26 [9,10] for extra power supply and connect the OTG with U-disk to the J27.
You will get a file named btsnoop on the U-disk.You can change its extension to .cfa , then open it with ComProbe Protocol Analysis System to view the HCI logs.
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
The log below shows the output of the example in the terminal window. 

Note:
1. Please note that whether the commands described in the document are supported depends on the specific hardware. Please use the command "help" to view the specific commands supported by the example.
2. The shell information "SHELL build: Aug 10 2021" may be different, which depends on the compile date.
3. Please note that not all cases of shell project have been verified. Only verified cases are listed in the readme file.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BLE shell demo start...

SHELL build: Aug 10 2021
Copyright  2020  NXP

@bt>
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The shell command list could be gotten by entering "help" in serial terminal.
The demo can be configed as a "central" or "peripheral" by shell commands.

Here is an example of central, scan devices (the BLE host must initialized before):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@bt> bt.init
@bt> Bluetooth initialized
Settings Loaded

@bt> bt.scan on
Bluetooth active scan enabled
@bt> [DEVICE]: 44:6D:F5:85:DC:5F (random), AD evt type 0, RSSI -64  C:1 S:1 D:0 SR:0 E:0 Prim: LE 1M, Secn: No packets, Interval: 0x0000 (0 ms), SID: 0xff
[DEVICE]: 44:6D:F5:85:DC:5F (random), AD evt type 4, RSSI -63  C:0 S:1 D:0 SR:1 E:0 Prim: LE 1M, Secn: No packets, Interval: 0x0000 (0 ms), SID: 0xff
[DEVICE]: 6D:B3:D3:8E:ED:A2 (random), AD evt type 0, RSSI -77  C:1 S:1 D:0 SR:0 E:0 Prim: LE 1M, Secn: No packets, Interval: 0x0000 (0 ms), SID: 0xff
[DEVICE]: 6D:B3:D3:8E:ED:A2 (random), AD evt type 4, RSSI -76  C:0 S:1 D:0 SR:1 E:0 Prim: LE 1M, Secn: No packets, Interval: 0x0000 (0 ms), SID: 0xff
[DEVICE]: 3F:FB:95:F7:F9:14 (random), AD evt type 3, RSSI -75  C:0 S:0 D:0 SR:0 E:0 Prim: LE 1M, Secn: No packets, Interval: 0x0000 (0 ms), SID: 0xff
[DEVICE]: 49:A3:4E:86:63:0C (random), AD evt type 0, RSSI -76  C:1 S:1 D:0 SR:0 E:0 Prim: LE 1M, Secn: No packets, Interval: 0x0000 (0 ms), SID: 0xff
[DEVICE]: 49:A3:4E:86:63:0C (random), AD evt type 4, RSSI -75  C:0 S:1 D:0 SR:1 E:0 Prim: LE 1M, Secn: No packets, Interval: 0x0000 (0 ms), SID: 0xff
[DEVICE]: 5C:28:50:F9:DD:57 (random), AD evt type 0, RSSI -82  C:1 S:1 D:0 SR:0 E:0 Prim: LE 1M, Secn: No packets, Interval: 0x0000 (0 ms), SID: 0xff
[DEVICE]: 4A:7D:B4:12:7B:7A (random), AD evt type 0, RSSI -82  C:1 S:1 D:0 SR:0 E:0 Prim: LE 1M, Secn: No packets, Interval: 0x0000 (0 ms), SID: 0xff
[DEVICE]: 4A:7D:B4:12:7B:7A (random), AD evt type 4, RSSI -82  C:0 S:1 D:0 SR:1 E:0 Prim: LE 1M, Secn: No packets, Interval: 0x0000 (0 ms), SID: 0xff
[DEVICE]: 5A:54:C8:99:13:4A (random), AD evt type 0, RSSI -76  C:1 S:1 D:0 SR:0 E:0 Prim: LE 1M, Secn: No packets, Interval: 0x0000 (0 ms), SID: 0xff
[DEVICE]: 3B:95:00:4D:F3:EB (random), AD evt type 3, RSSI -82  C:0 S:0 D:0 SR:0 E:0 Prim: LE 1M, Secn: No packets, Interval: 0x0000 (0 ms), SID: 0xff
[DEVICE]: 47:9D:D0:CB:5F:0D (random), AD evt type 0, RSSI -86  C:1 S:1 D:0 SR:0 E:0 Prim: LE 1M, Secn: No packets, Interval: 0x0000 (0 ms), SID: 0xff
@bt> bt.scan off
Scan successfully stopped
@bt>
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Here is a example of advertise (the BLE host must initialized before):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
@bt> bt.init
@bt> Bluetooth initialized

@bt> bt.advertise on
Advertising started
@bt> bt.advertise off
Advertising stopped
@bt> 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

NOTE. the mentioned "command complete event" can be found in HCI log, U-DISK should be connected to usb port to get HCI log capture. CONFIG_BT_SNOOP macro is used to enable stack capture the HCI log.

Here is the log of rf_test_mode application:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
>> help

@bt> help
help

+---"help": List all the registered commands
+---"exit": Exit program
+---"echo": Set echo(0 - disable, 1 - enable)
+---"bt": bt "command entry"
    +---"init": init "[no-settings-load], [sync]"
    +---"settings-load": settings-load "[none]"
    +---"id-create": id-create "[addr]"
.............................
    +---"ca_test": ca_test "<test cover art>"
+---"bt_test": bt_test "Bluetooth BR/EDR test mode commands"
    +---"tx_test": tx_test "test_scenario[1] hopping_mode[1] tx_channel[1] rx_channel[1] tx_test_interval[1] pkt_type[1] data_length[2] whitening[1] num_pkt[4] tx_pwr[1]"
    +---"rx_test": rx_test "test_scenario[1] tx_channel[1] rx_channel[1] pkt_type[1] num_pkt[4] data_length[2] tx_addr[6] report_err_pkt[1]"
    +---"reset": reset " Reset the HCI interface"
+---"le_test": le_test "Bluetooth BLE test mode commands"
    +---"tx_test": tx_test "tx_channel[1] data_length[1] payload[1] phy[1]"
    +---"rx_test": rx_test "rc_channel[1] phy[1] modulation[1]"
    +---"end_test": end_test "end the le test"
@bt> 
>>
>> bt.init
bt.init

>>
>> Bluetooth initialized
Settings Loaded

>>
To start the transmitter test packets for Bluetooth Classic:
@bt> bt_test.tx_test 01 01 01 01 0D 03 0F 00 00 00 00 00 00 04

rx_on_start default set to=80

synt_on_start default set to=80

tx_on_start default set to=80

phd_off_start default set to=80

test_scenario= 1

hopping_mode= 1

tx_channel= 1

rx_channel= 1

tx_test_interval= d

pkt_type= 3

data_length= f 0

whitening= 0

num_pkt= 0 0 0 0

tx_pwr= 4

@bt> API returned success...

Observe the DM1 packets in over the air logs. 

To stop the transmitter test packets for Bluetooth Classic:

@bt>  bt_test.tx_test FF 01 01 01 0D 03 0F 00 00 00 00 00 00 04

rx_on_start default set to=80

@bt> synt_on_start default set to=80

tx_on_start default set to=80

phd_off_start default set to=80


test_scenario= ff


hopping_mode= 1


tx_channel= 1

rx_channel= 1

tx_test_interval= d

pkt_type= 3

data_length= f 0

whitening= 0

num_pkt= 0 0 0 0

tx_pwr= 4

API returned success...

Observe that DM1 packets are now stopped transmitting in over the air logs. 

To perform HCI reset 
@bt> bt_test.reset
API returned success...
>>
>>

To start the receiving test packets for Bluetooth Classic:


@bt> bt_test.rx_test 01 01 01 03 10 00 00 00 0F 00 20 4E F6 EC 1F 26 00
test_scenario= 1

tx_channel= 1


rx_channel= 1

pkt_type= 3

num_pkt= 10 0 0 0

data_length= f 0

tx_am_addr default set to= 1

tx_addr: 
20 
4e 
f6 
ec 
1f 
26 


report_err_pkt= 0

@bt> API returned success...

To stop receiving test packets for Bluetooth Classic:

@bt> bt_test.rx_test FF 01 01 03 10 00 00 00 0F 00 20 4E F6 EC 1F 26 00

test_scenario= ff

tx_channel= 1

rx_channel= 1
@bt> 
pkt_type= 3

num_pkt= 10 0 0 0

data_length= f 0

tx_am_addr default set to= 1

tx_addr: 
20 

4e 
f6 
ec 

1f 
26 


report_err_pkt= 0

API returned success...

Observe the packet count in vendor-specific command complete event.  

To start the transmitter test packets for Bluetooth LE:

@bt> le_test.tx_test 01 FF 00 01

tx_channel= 1

test_data_len= ff

pkt_payload= 0

phy= 1

@bt> API returned success...

Observe the transmitter test packets in over the air logs.

To stop the transmitter test packets for Bluetooth LE:
@bt> le_test.end_test
API returned success...
>>

To start the receiving test packets for Bluetooth LE:
@bt> le_test.rx_test 01 01 00
rx_channel= 1

phy= 1

modulation_index= 0

@bt> API returned success...

le_test.end_test
API returned success...
>>
Observe the packet count in command complete event. 


Running a2dp
The commands are as follow:
+---"a2dp": a2dp Bluetooth A2DP shell commands
    +---"register_sink_ep": register_sink_ep <select codec.
			1:SBC
			2:MPEG-1,2
			3:MPEG-2,4
			4:vendor
			5:sbc with delay report and content protection services
			6:sbc with all other services(don't support data transfer yet)>
    +---"register_source_ep": register_source_ep <select codec.
			1:SBC
			2:MPEG-1,2
			3:MPEG-2,4
			4:vendor
			5:sbc with delay report and content protection services
			6:sbc with all other services(don't support data transfer yet)>
    +---"connect": connect [none]
    +---"disconnect": disconnect [none]
    +---"configure": configure [none]
    +---"discover_peer_eps": discover_peer_eps [none]
    +---"get_registered_eps": get_registered_eps [none]
    +---"set_default_ep": set_default_ep <select endpoint>
    +---"configure_ep": configure_ep "configure the default selected ep"
    +---"deconfigure": deconfigure "de-configure the default selected ep"
    +---"start": start "start the default selected ep"
    +---"stop": stop "stop the default selected ep"
    +---"send_media": send_media <second> "send media data to the default selected ep"
Test flow:
1 Create ACL connection between two devices (A and B).
2 In device B, input "a2dp.register_sink_ep x" to initialize sink endpoint.
3 In device A, input "a2dp.register_source_ep x" to initialize source endpoint.
4 In device A, input "a2dp.connect" to create a2dp connection with the default ACL connection.
5 In device A, input "a2dp.configure" to configure the a2dp connection.
6 In device A, input "a2dp.start" to start the a2dp media.
7 In device A, input "a2dp.send_media x" to send media data for x seconds.
8 For other commands:
   8.1 "a2dp.disconnect" is used to disconnect the a2dp.
   8.2 "a2dp.discover_peer_eps" is used to discover peer device's endpoints.
   8.3 "a2dp.get_registered_eps" is used to get the local registered endpoints.
   8.4 "a2dp.set_default_ep" is used to set the default selected endpoint.
   8.5 "a2dp.deconfigure" de-configure the endpoint, then it can be configured again.
   8.6 "a2dp.stop" stops media.
   8.7 "a2dp.send_delay_report" send delay report.

Running avrcp
The commands are as follow:
+---"avrcp": avrcp Bluetooth AVRCP shell commands
    +---"init_ct": init_ct [none]
    +---"init_tg": init_tg [none]
    +---"ctl_connect": ctl_connect "create control connection"
    +---"brow_connect": brow_connect "create browsing connection"
    +---"ct_list_all_cases": ct_list_all_cases "display all the test cases"
    +---"ct_test_case": ct_test_case <select one case to test>
    +---"ct_test_all": ct_test_all "test all cases"
    +---"ct_reg_ntf": ct_reg_ntf <Register Notification. select event:
                                    1. EVENT_PLAYBACK_STATUS_CHANGED
                                    2. EVENT_TRACK_CHANGED
                                    3. EVENT_TRACK_REACHED_END
                                    4. EVENT_TRACK_REACHED_START
                                    5. EVENT_PLAYBACK_POS_CHANGED
                                    6. EVENT_BATT_STATUS_CHANGED
                                    7. EVENT_SYSTEM_STATUS_CHANGED
                                    8. EVENT_PLAYER_APPLICATION_SETTING_CHANGED
                                    9. EVENT_NOW_PLAYING_CONTENT_CHANGED
                                    a. EVENT_AVAILABLE_PLAYERS_CHANGED
                                    b. EVENT_ADDRESSED_PLAYER_CHANGED
                                    c. EVENT_UIDS_CHANGED
                                    d. EVENT_VOLUME_CHANGED>
    +---"tg_notify": tg_notify <Notify event. select event:
                                    1. EVENT_PLAYBACK_STATUS_CHANGED
                                    2. EVENT_TRACK_CHANGED
                                    3. EVENT_TRACK_REACHED_END
                                    4. EVENT_TRACK_REACHED_START
                                    5. EVENT_PLAYBACK_POS_CHANGED
                                    6. EVENT_BATT_STATUS_CHANGED
                                    7. EVENT_SYSTEM_STATUS_CHANGED
                                    8. EVENT_PLAYER_APPLICATION_SETTING_CHANGED
                                    9. EVENT_NOW_PLAYING_CONTENT_CHANGED
                                    a. EVENT_AVAILABLE_PLAYERS_CHANGED
                                    b. EVENT_ADDRESSED_PLAYER_CHANGED
                                    c. EVENT_UIDS_CHANGED
                                    d. EVENT_VOLUME_CHANGED>
    +---"ca_init_i": ca_init_i "Init cover art initiator"
    +---"ca_init_r": ca_init_r "Init cover art responder"
    +---"ca_connect": ca_connect "create cover art connection"
    +---"ca_test": ca_test "cover art test all cases"
Test flow:
1 Create ACL connection between two devices (A and B).
2 In device B, input "avrcp.init_tg" to initialize Target.
3 In device A, input "avrcp.init_ct" to initialize Controller.
4 In device B, input "avrcp.ca_init_r" to initialize Cover Art responder.
5 In device A, input "avrcp.ca_init_i" to initialize Cover Art Initiator.
6 In device A, input "avrcp.ctl_connect" to create AVRCP Control connection.
7 In device A, input "avrcp.brow_connect" to create AVRCP Browsing connection.
8 In device A, input "avrcp.ct_test_all" to test all the cases.
9 In device A, input "avrcp.ct_reg_ntf" to register notification.
10 In device A, input "avrcp.ca_connect" to create AVRCP Cover Art connection.
11 In device B, input "avrcp.tg_notify" to notify.
12 In device A, input "avrcp.ca_test" to test all the cover art commands.
13 For other commands:
   13.1 In device A, input "avrcp.ct_list_all_cases" to list all the test cases.
   13.2 In device A, input "avrcp.ct_test_case x" to test one selected case.

Running BR/EDR L2CAP
Test L2CAP basic mode
1 Create ACL connection between two devices (A and B).
2 In device A and B, input "br.l2cap-register <psm>" to register one psm (for example: br.l2cap-register 1001).
3 In device A, input "br.l2cap-connect <psm>" to create l2cap connection (for example: br.l2cap-connect 1001).
4 In device A, input "br.l2cap-send x" to send data.
5 In device A, input "br.l2cap-disconnect" to disconnect the l2cap connection.
Teset L2CAP Retransmission and Streaming Mode
1 Create ACL connection between two devices (A and B).
2 In device A and B, input "br.l2cap-register-mode <psm>" to register one psm (for example: br.l2cap-register-mode 1001).
3 In device A, input "br.l2cap-connect <psm>" to create l2cap connection (for example: br.l2cap-connect 1001).
4 In device A, input "br.l2cap-send x" to send data.
5 In device A, input "br.l2cap-disconnect" to disconnect the l2cap connection.

Here is a example of BLE pairing and bonding,
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
GATT peripheral role side,
1. Initialize the Host, press "bt.init",
2. Advertising, press "bt.advertise on",
3. After the connection is established, perform the pairing sequence,
   it could be started from peripheral side by pressing "bt.security <level>", such as "bt.security 2".
4. If the bondable is unsupported by peripheral role, press "bt.bondable off". Then start step 3.

GATT central role side,
1. Initialize the Host, press "bt.init",
2. Scaning advertising packets, press "bt.scan on",
3. A few seconds later, stop the scanning, press "bt.scan off"
4. Select the target board and create a new connection. If the taregt is not listed, repeat steps 2 and 3.
   Then press "bt.connect <address: XX:XX:XX:XX:XX:XX> <type: (public|random)>"
5. After the connection is established, perform the pairing sequence,
   it could be started from central side by pressing "bt.security <level>", such as "bt.security 2".
6. If the bondable is unsupported by central role, press "bt.bondable off". Then start step 5.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Here is a example of GATT data signing,
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
GATT peripheral role side,
1. Initialize the Host, press "bt.init",
2. Advertising, press "bt.advertise on",
3. After the connection is established, perform the pairing sequence,
   it could be started from peripheral side by pressing "bt.security <level>", such as "bt.security 2",
4. After the authentication is successfully, disconnect the connection,
   it could be started from peripheral side by pressing "bt.disconnect",
5. Waiting for new connection. After the connection is established (LL enceyption should be disabled),
   add new serivce "gatt.register".

GATT central role side,
1. Initialize the Host, press "bt.init",
2. Scaning advertising packets, press "bt.scan on",
3. A few seconds later, stop the scanning, press "bt.scan off"
4. Select the target board and create a new connection. If the taregt is not listed, repeat steps 2 and 3.
   Then press "bt.connect <address: XX:XX:XX:XX:XX:XX> <type: (public|random)>"
5. After the connection is established, perform the pairing sequence,
   it could be started from central side by pressing "bt.security <level>", such as "bt.security 2",
6. After the authentication is successfully, disconnect the connection,
   it could be started from central side by pressing "bt.disconnect",
7. Repeat the steps 2 and 3. After the connection is established (LL enceyption should be disabled),
   perform the GATT data signing sequence, press "gatt.signed-write <handle> <data> [length] [repeat]",
   such as "gatt.signed-write 22 AA 1"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Here is a example of GATT Service Changed Indication,
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
GATT peripheral role side,
1. Initialize the Host, press "bt.init",
2. Advertising, press "bt.advertise on",
3. After the connection is established. and waiting for the service changed indication is subsribed,
4. Add new serivce, press "gatt.register",
5. Remove the added serivce, press "gatt.unregister".

GATT central role side,
1. Initialize the Host, press "bt.init",
2. Scaning advertising packets, press "bt.scan on",
3. A few seconds later, stop the scanning, press "bt.scan off"
4. Select the target board and create a new connection. If the taregt is not listed, repeat steps 2 and 3.
   Then press "bt.connect <address: XX:XX:XX:XX:XX:XX> <type: (public|random)>"
5. After the connection is established, subscribe the GATT service changed indicator. press "bt.subscribe <CCC handle> <value handle> [ind]",
   such as "gatt.subscribe f e ind".
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Here is a example of GATT Service Dynamic Database Hash,
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
GATT peripheral role side,
1. Initialize the Host, press "bt.init",
2. Advertising, press "bt.advertise on",
3. After the connection is established. and waiting for the service changed indication is subsribed,
4. Add new serivce, press "gatt.register",
5. Remove the added serivce, press "gatt.unregister".

GATT central role side,
1. Initialize the Host, press "bt.init",
2. Scaning advertising packets, press "bt.scan on",
3. A few seconds later, stop the scanning, press "bt.scan off"
4. Select the target board and create a new connection. If the taregt is not listed, repeat steps 2 and 3.
   Then press "bt.connect <address: XX:XX:XX:XX:XX:XX> <type: (public|random)>"
5. After the connection is established, subscribe the GATT service changed indicator. press "bt.subscribe <CCC handle> <value handle> [ind]",
   such as "gatt.subscribe f e ind".
6. If the indication is indicated, read DB hash, press "gatt.read <handle> [offset]" or "gatt.read-uuid <UUID> [start handle] [end handle]", 
   such as "gatt.read 13", or "gatt.read-uuid 2b2a".

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Here is an example of filter accept list.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

GATT peripheral role side,
1. Initialize the Host, press "bt.init",
2. Adding device to white list, press "bt.fal-add <address: XX:XX:XX:XX:XX:XX> <type: (public|random)>",such as "bt.fal-add 11:22:33:44:55:66 public".
3. Advertising, press "bt.advertise on fal-conn",
4. Only the device in filter accept list can connect to current device. or else no log will be print.

Note: if device address is added after command bt.advertise on, then filter accept list will take effect after re-star advertise.
the bt.advertise off and bt.advertise on can be used to re-start the advertise. 

GATT central role side,
1. Initialize the Host, press "bt.init",
2. Adding device to filter accept list, press "bt.fal-add <address: XX:XX:XX:XX:XX:XX> <type: (public|random)>",such as "bt.fal-add 80:D2:1D:E8:2B:7E public".
3. press "bt.fal-connect on".
@bt> Connected: 80:D2:1D:E8:2B:7E (public)
4. press "bt.disconnect". device will be disconnect.
@bt> Disconnected: 80:D2:1D:E8:2B:7E (public) (reason 0x16)
4. bt.fal-rem 80:D2:1D:E8:2B:7E public

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Here is an example of 1M/2M/Coded PHY update.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
GATT peripheral role side,
1. Initialize the Host, press "bt.init",
2. Advertising, press "bt.advertise on",
3. After the connection is established.
4. Send phy update command, press "bt.phy-update <tx_phy> [rx_phy] [s2] [s8]", tx_phy/rx_phy could be 1(1M) or 2(2M) or 4(Coded). 
   such as "bt.phy-update 2 2".
5. The message "LE PHY updated: TX PHY LE 2M, RX PHY LE 2M" would be printed if the phy is updated. note, if peer don't support phy update, then this message will not be printed.

GATT central role side,
1. Initialize the Host, press "bt.init",
2. start scan, press "bt.scan on", Bluetooth device around your current bluetooth will be list, for example, 
[DEVICE]: 72:78:C1:B5:0F:DA (random), AD evt type 4, RSSI -32 BLE Peripheral C:0 S:1 D:0 SR:1 E:0 Prim: LE 1M, Secn: No packets, Interval: 0x0000 (0 ms), SID: 0xff
[DEVICE]: C4:0D:02:55:5E:AD (random), AD evt type 0, RSSI -83  C:1 S:1 D:0 SR:0 E:0 Prim: LE 1M, Secn: No packets, Interval: 0x0000 (0 ms), SID: 0xff
[DEVICE]: 66:8F:26:27:1F:52 (random), AD evt type 0, RSSI -82  C:1 S:1 D:0 SR:0 E:0 Prim: LE 1M, Secn: No packets, Interval: 0x0000 (0 ms), SID: 0xff
3. stop scan, press "bt.scan off",
4. connect target device, press "bt.connect <address: XX:XX:XX:XX:XX:XX> <type: (public|random)>", such as bt.connect 72:78:C1:B5:0F:DA random
5. Send phy update command, press "bt.phy-update <tx_phy> [rx_phy] [s2] [s8]", tx_phy/rx_phy could be 1(1M) or 2(2M) or 4(Coded). 
   such as "bt.phy-update 2 2".
6. The message "LE PHY updated: TX PHY LE 2M, RX PHY LE 2M" would be printed if the phy is updated. note, if peer don't support phy update, then this message will not be printed.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Here is an example of LE Data Packet Length Extension update.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
GATT peripheral role side,
1. Initialize the Host, press "bt.init".
2. Advertising, press "bt.advertise on".
3. After the connection is established.
4. Check current LE RX/TX maximum data length and time, press "bt.info", as blow, default RX/TX maximum data length is 27 and default RX/TX maxumum time is 328.
Type: LE, Role: slave, Id: 0
59:8F:3C:20:93:86 (random)
Remote address: 59:8F:3C:20:93:86 (random) (resolvable)
Local address: 80:D2:1D:E8:30:EC (public) (identity)
Remote on-air address: 59:8F:3C:20:93:86 (random) (resolvable)
Local on-air address: 7C:59:48:2E:A4:51 (random) (resolvable)
Interval: 0x0024 (45 ms)
Latency: 0x0000 (0 ms)
Supervision timeout: 0x0190 (4000 ms)
LE PHY: TX PHY LE 1M, RX PHY LE 1M
LE data len: TX (len: 27 time: 328) RX (len: 27 time: 328)
5. When LE data len is updated by the peer device, below information will be printed.
LE data len updated: TX (len: 27 time: 328) RX (len: 50 time: 512)
6. Update maximum tx data length, press "bt.data-len-update <tx_max_len> [tx_max_time]", such as bt.data-len-update 65, below information will be printed.
Calculated tx time: 632
59:8F:3C:20:93:86 (random)
data len update initiated.
LE data len updated: TX (len: 65 time: 632) RX (len: 50 time: 512)

GATT central role side,
1. Initialize the Host, press "bt.init".
2. Start scan, press "bt.scan on", Bluetooth device around your current bluetooth will be list, for example, 
[DEVICE]: 7C:59:48:2E:A4:51 (random), AD evt type 4, RSSI -44 BLE Peripheral C:0 S:1 D:0 SR:1 E:0 Prim: LE 1M, Secn: No packets, Interval: 0x0000 (0 ms), SID: 0xff
3. Stop scan, press "bt.scan off",
4. Connect target device, press "bt.connect <address: XX:XX:XX:XX:XX:XX> <type: (public|random)>", such as bt.connect 7C:59:48:2E:A4:51 random
5. Check current LE RX/TX maximum data length and time, press "bt.info", as blow, default RX/TX maximum data length is 27 and default RX/TX maxumum time is 328.
Type: LE, Role: master, Id: 0
7C:59:48:2E:A4:51 (random)
Remote address: 7C:59:48:2E:A4:51 (random) (resolvable)
Local address: C0:95:DA:00:BC:82 (public) (identity)
Remote on-air address: 7C:59:48:2E:A4:51 (random) (resolvable)
Local on-air address: 59:8F:3C:20:93:86 (random) (resolvable)
Interval: 0x0024 (45 ms)
Latency: 0x0000 (0 ms)
Supervision timeout: 0x0190 (4000 ms)
LE PHY: TX PHY LE 1M, RX PHY LE 1M
LE data len: TX (len: 27 time: 328) RX (len: 27 time: 328)
6. Update maximum tx data length, press "bt.data-len-update <tx_max_len> [tx_max_time]", such as bt.data-len-update 50, below information will be printed.
Calculated tx time: 512
7C:59:48:2E:A4:51 (random)
data len update initiated.
LE data len updated: TX (len: 50 time: 512) RX (len: 27 time: 328)
7. When LE data len is updated by the peer device, below information will be printed.
LE data len updated: TX (len: 50 time: 512) RX (len: 65 time: 632)

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Running BR/EDR RFCOMM
Note: Only 1 rfcomm connection is supported in shell project.

RFCOMM Server Side,
1. Input "bt.init" to initialize bluetooth
2. Input "br.pscan on" to turn on pscan
2. Input "br.iscan on" to turn on iscan
3. Input "rfcomm.register 5" to register rfcomm server channel 5
4. After rfcomm connection is created, input "rfcomm.send <count of sending>" to send data
5. After rfcomm connection is created, input "rfcomm.disconnect" to disconnect with peer device

RFCOMM Client Side,
1. Input "bt.init" to initialize bluetooth
2. Input "br.discovery on" to turn on discovery
2. Input "br.connect <address>" to create br connection, for example: br.connect 80:D2:1D:E8:2B:7E
3. Input "rfcomm.connect 5" to create rfcomm connection on channel 5
4. After rfcomm connection is created, input "rfcomm.send <count of sending>" to send data
5. After rfcomm connection is created, input "rfcomm.disconnect" to disconnect with peer device

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Running BR/EDR PBAP
Note: Only 1 PBAP connection is supported in shell project.
shell project supports PCE and PSE.

The commands are as follow:
+---"pbap": pbap Bluetooth pbap shell commands
    +---"pce": pce [none]
        +---"register": register [none]
        +---"connect": connect SDP first, then connect.
                            -psm(optional).
                            obex auth params(optional)
                            -uid   : [userid].
                            -pwd   : [password].
        +---"disconnect": disconnect [none]
        +---"abort": abort [none]
        +---"pull_phonebook": pull_phonebook 
                           -name(mandatory) : [name].
                            -srmp(optional)  : [Single Response Mode Param(>=0)].
                           input application parameters(optional). 
                           1: -ps  : [Property Selector (64-bit)].
                           2: -f   : [Format(0: vcard 2.1 | 1 : vcard 3.0)].
                           3: -mlc : [MaxListCount (0 - 0xFFFF)].
                           4: -lso : [ListStartOffset (0 - 0xFFFF)].
                           5: -rnmc: [RestNewMissedCalls(0/1)].
                           6: -cs  : [vCardSelector(64-bit)].
                           7: -cso : [vCardSelecorOperator(0 : or | 1 : and)]
        +---"set_path": set_path [path_name]
        +---"pull_vcardlist": pull_vcardlist 
                            -name(mandatory) : [name].
                            -srmp(optional)  : [Single Response Mode Param(>=0)].
                           input application parameters(optional). 
                           1: -o   : [order(0 : Indexed | 1 : Alphanumeric | 2 : Phonetical)].
                           2: -sp  : [SearchProperty(0 : name | 1 : number | 2 : sound)].
                           3: -sv  : [SearchValue(string)].
                           4: -mlc : [MaxListCount (0 - 0xFFFF)].
                           5: -lso : [ListStartOffset (0 - 0xFFFF)].
                           6: -rnmc: [ResetNewMissedCalls(0/1)].
                           7: -cs  : [vCardSelector (64-bit)].
                           8: -cso : [vCardSelecorOperator(0 : or | 1 : and)].
        +---"pull_vcardentry": pull_vcardentry 
                            -name(mandatory) : [name].
                            -srmp(optional)  : [Single Response Mode Param(>=0)].
                           input application parameters(optional).
                           1: -ps  : [Property Selector (64-bit)].
                           2: -f   : [Format(0: vcard 2.1 | 1 : vcard 3.0)].
    +---"pse": pse [none]
        +---"register": register [none]
        +---"disconnect": disconnect [none]

Here is an example of run PCE, and PSE is a mobile phone or a board running a PSE application.

1. Input "bt.init" to initialize bluetooth
2. After bluetooth init successfully, Input "br.pscan on" to turn on pscan, PCE can be discoverable.
3. Input "br.iscan on" to turn on iscan, PCE can be connectable.
4. Input "pbap.pce.register" to init PBAP PCE.
5. Input "hfp.init" to init HFP.
   If PSE is a mobile phone and it wants to connect PCE proactively, this step is mandatory, otherwise optional.
6. Input "br.discovery on" to discover PSE.
   If PSE connect PCE proactively, this step is optional.
7. Input "br.connect xx:xx:xx:xx:x:xx" to create ACL connection to PSE.
   If PSE connect PCE proactively, do not perform this step.
8. After ACL connected successfully, input "pbap.pce.connect" to create PBAP connection to PSE.
   PCE establishes PBAP connections to PSE based on the rfcomm channel by default.
   If want to based on GOEP_L2CAP_PSM, add parameter "-psm", like "pbap.connect -psm".
   If PSE does not supported GOEP_L2CAP_PSM, PCE will continue to establishe PBAP connection based on the rfcomm channel.
   If PCE wants to authenticate PSE proactively, add parameter "-pwd" which means OBEX pin_code, like "pbap.connect -pwd 0000".
   parameter '-uid' which means OBEX use_id is optional because it is not suppoeted now.
   If PSE and PCE have not negotiated authenticate information, it is recommended that PCE do not authenticate PSE proactively.
   For example: PSE is a mobile phone.
9. After PBAP connected successfully, input "pbap.pce.pull_phonebook -name telecom/pb.vcf" to pull phonebook object from PSE.
   The name shall contain the absolute path in the virtual folders architecture of the PSE.
   Example: telecom/pb.vcf or SIM1/telecom/pb.vcf.
   If want to set the parameter "srmp", add the parameter "-srmp", like "pbap.pull_phonebook -name telecom/pb.vcf -srmp 3". 
   If want to add applicaton parameters, input like "pbap.pull_phonebook -name telecom/pb.vcf -f 1".
   All application parameters are optional and their specific information can be found in spec.
   It is possible that PSE does not support all parameters. 
10. After receiving all the information from PSE, input "pbap.pce.set_path ./telecom" to set PSE path to telecom.
   When name is "/", go to root directory.
   When name is ".." or "../", go up one level.
   When name is "child" or "./child", go to child.
11. After setting path successfully, input "pbap.pce.pull_vcardlist -name cch" to pull cch vcard listing object from PSE.
   The name shall not include any path information, since the PullvCardListing function uses relative paths.
   Example: cch or ich or och.
   If want to set the parameter "srmp", add the parameter "-srmp", like "pbap.pull_vcardlist -name cch -srmp 3". 
   If want to add applicaton parameters, input like "pbap.pull_vcardlist -name cch -name telecom/pb.vcf -o 0".
   All application parameters are optional and their specific information can be found in spec.
   It is possible that PSE does not support all parameters or name(spd, fav). 
12. After receiving all the information from PSE, Input "pbap.pce.set_path ./cch" to set pse path to cch
   When name is "/", go to root directory.
   When name is ".." or "../", go up one level.
   When name is "child" or "./child", go to child.
13. After setting path successfully, Input "pbap.pce.pull_vcardentry -name 4.vcf" to pull handle=4 vcard entry object from PSE.
   The name shall be used to indicate the name or the X-BT-UID of the object, and not include any path information.
   Example: 4.vcf or X-BT-UID:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX.
   If want to set the parameter "srmp", add the parameter "-srmp", like "pbap.pull_vcardentry -name 4.vcf -srmp 3". 
   If want to add applicaton parameters, input like "pbap.pull_vcardentry -name 4.vcf -f 1".
   All application parameters are optional and their specific information can be found in spec.
   It is possible that PSE does not support all parameters or X-BT-UID. 
14. Input "pbap.pce.disconnect" to disconnect PBAP connection to PSE
   If PSE is a mobile phone, it will disconnect ACL connect proactively after pbap disconnect.

Note:
1. If using the a mobile phone as PSE to actively connect to PCE, please be sure to follow step 5 and ignore step 6, 7.
   After testing, it was found that some mobile phones would only actively query for major services such as HFP or A2DP etc,
   and if it did not find them or establish an L2CAP connection within the allotted time, the phone would actively disconnect ACL connection.
   If the PCE initiates the connection to a mobile phone actively, step 5 is optional.
2. If PSE is a mobile iphone, after connection,  user needs to actively turn on the "Share Contacts Permission" on the phone.
   Otherwise PCE can not get message from PSE.
3. If you want to get the vcard listing object or vcard entry object which belongs to a certain folder, please set path to the certain folder first.
   Example： if you want to get vcard entry object "telecom/cch/4.vcf", first set path to "telecom", then set path to "cch".
4. It is possible that PSE does not support all application parameters.

Here is an example of run PSE, and PCE is a board running a shell project.

1. Input "bt.init" to initialize bluetooth.
2. After bluetooth init successfully, Input "br.pscan on" to turn on pscan, PSE can be discoverable.
3. Input "br.iscan on" to turn on iscan, PSE can be connectable.
4. Input "pbap.pse.register" to init PBAP PSE.
5. After that, on PCE side, perform the above steps(step 1 - 14) to test PSE functions.
6. Input "pbap.pse.disconnect" to disconnect PBAP connection.
   PBAP connection also can be disconnected by inputting "pbap.pce.disconnect" on PCE side.

Note:
1. This example acting as PSE doesn't supports all application parameters and only supports to parse/send the part of application parameters from/to PCE.

Running BR/EDR MAP
Note: Only 1 MAP MAS and MNS connection is supported in shell project.
shell project supports MCE and MSE.

The commands are as follow:
+---"map": map Bluetooth MAP shell commands
    +---"mce": mce [none]
        +---"register": register [none]
        +---"unregister": unregister [none]
        +---"mns_register": mns_register [none]
        +---"mns_unregister": mns_unregister [none]
        +---"connect": connect SDP first, then connect
        +---"disconnect": disconnect [none]
        +---"mns_disconnect": mns_disconnect [none]
        +---"abort": abort [none]
        +---"get_folder_list": get_folder_list 
               -srmp(optional) : [Single Response Mode Param (>=0)].
               input application parameters(optional).
               1: -mlc : [MaxListCount (0 - 0xFFFF)].
               2: -lso : [ListStartOffset (0 - 0xFFFF)].
        +---"set_folder": set_folder 
               -name(mandatory) : [name ("/" : root | "../" : parent | "child" : child | "../child" : parent then child)].
        +---"get_msg_list": get_msg_list 
               -name(mandatory if getting child folder, or optional) : [name (string)].
               -srmp(optional) : [Single Response Mode Param (>=0)].
               input application parameters(optional).
               1: -mlc : [MaxListCount (0 - 0xFFFF)].
               2: -lso : [ListStartOffset (0 - 0xFFFF)].
               3: -sl  : [SubjectLength (1 - 255)].
               4: -pm  : [ParameterMask (0 - 0x1FFFFF)].
               5: -fmt : [FilterMessageType (0 - 0x1F)].
               6: -fpb : [FilterPeriodBegin (string of timestamp)].
               7: -fpe : [FilterPeriodEnd (string of timestamp)].
               8: -frs : [FilterReadStatus (0 : no-filter | 1: unread | 2 : read)].
               9: -fr  : [FilterRecipient (string)].
               10: -fo : [FilterOriginator (string)].
               11: -fp : [FilterPriority (0 : no-filter | 1: high priority msg | 2 : non-high priority msg)].
               12: -ci : [ConversationID (128-bit value in hex string format)].
               13: -fmh : [FilterMessageHandle (64-bit value in hex string format)].
        +---"get_msg": get_msg 
               -name(mandatory) : [MessageHandle (string)].
               -srmp(optional)  : [Single Response Mode Param (>=0)].
               input application parameters.
               1: -a(mandatory) : [Attachment (0 : OFF | 1 : ON)].
               2: -c(mandatory) : [Charset (0 : native | 1 : UTF-8)].
               3: -fr(optional) : [FractionRequest (0 : first | 1 : next)].
        +---"set_msg_status": set_msg_status 
               -name(mandatory) : [Message Handle (string)].
               input application parameters.
               1: -si(mandatory) : [StatusIndicator (0 : readStatus | 1 : deletedStatus | 2 : setExtendedData)].
               2: -sv(mandatory) : [StatusValue (0 : no | 1 : yes)].
               3: -ed(optional)  : [ExtendedData (string)].
        +---"push_msg": push_msg 
               -name(mandatory if pushing child folder, or optional) : [name (string)].
               input application parameters.
               1: -t(optional)  : [Transparent (0 : OFF | 1 : ON)].
               2: -r(optional)  : [Retry (0 : OFF | 1 : ON)].
               3: -c(mandatory) : [Charset (0 : native | 1 : UTF-8)].
               4: -ci(optional) : [ConversationID (128-bit value in hex string format)].
               5: -mh(optional if Message Forwarding is supported or excluded) : [MessageHandle (string)].
               6: -a(mandatory if MessageHandle present in request or excluded)  : [Attachment (0 : OFF | 1 : ON)].
               7: -mt(mandatory if MessageHandle present in request or excluded) : [ModifyText (0 : REPLACE | 1 : PREPEND)].
        +---"set_ntf_reg": set_ntf_reg 
               input application parameters(mandatory).
               1: -ns : [NotificationStatus (0 : OFF | 1 : ON)].
        +---"update_inbox": update_inbox [none]
        +---"get_mas_inst_info": get_mas_inst_info 
               -srmp(optional) : [Single Response Mode Param (>=0)].
               input application parameters(mandatory).
               1: -mii : [MASInstanceID (0 - 255)].
        +---"set_owner_status": set_owner_status 
               input application parameters(at least one parameter present).
               1: -pa : [PresenceAvailability (0 - 255)].
               2: -pt : [PresenceText (string)].
               3: -la : [LastActivity (string of timestamp)].
               4: -cs : [ChatState (0 - 255)].
               5: -ci : [ConversationID (128-bit value in hex string format)].
        +---"get_owner_status": get_owner_status 
               -srmp(optional) : [Single Response Mode Param (>=0)].
               input application parameters(optional).
               1: -ci : [ConversationID (128-bit value in hex string format)].
        +---"get_convo_list": get_convo_list 
               -srmp(optional) : [Single Response Mode Param (>=0)].
               input application parameters(optional).
               1: -mlc  : [MaxListCount (0 - 0xFFFF)].
               2: -lso  : [ListStartOffset (0 - 0xFFFF)].
               3: -flab : [FilterLastActivityBegin (string)].
               4: -flae : [FilterLastActivityEnd (string)].
               5: -frs  : [FilterReadStatus (0 : no-filter | 1: unread | 2 : read)].
               6: -fr   : [FilterRecipient (string)].
               7. -ci   : [ConversationID (128-bit value in hex string format)].
               8: -cpm  : [ConvParameterMask (0 - 0x7FFF)].
        +---"set_ntf_filter": set_ntf_filter 
               input application parameters(mandatory).
               1: -nfm : [NotificationFilterMask (0 - 0x7FFF)].
    +---"mse": mse [none]
        +---"register": register [none]
        +---"unregister": unregister [none]
        +---"mns_register": mns_register [none]
        +---"mns_unregister": mns_unregister [none]
        +---"disconnect": disconnect [none]
        +---"mns_disconnect": mns_disconnect [none]
        +---"send_event": send_event 
               input application parameters(mandatory).
               1: -mii : [MASInstanceID (0 - 255)].

Here is an example of run MCE, and MSE is a mobile phone or a board running a MSE application.

1. Input "bt.init" to initialize bluetooth.
2. After bluetooth init successfully, Input "br.pscan on" to turn on pscan, MCE can be discoverable.
3. Input "br.iscan on" to turn on iscan, MCE can be connectable.
4. Input "map.mce.register" to init MAP MCE MAS. Input "map.mce.mns_register" to init MAP MCE MNS.
5. Input "hfp.init" to init HFP.
   If MSE is a mobile phone and it wants to connect MCE proactively, this step is mandatory, otherwise optional.
6. Input "br.discovery on" to discover MSE.
   If MSE connects MCE proactively, this step is optional.
7. Input "br.connect xx:xx:xx:xx:xx:xx" to create ACL connection to MSE.
   If MSE connects MCE proactively, do not perform this step.
8. After ACL connected successfully, input "map.mce.connect" to create MAP MAS connection to MSE.
   MAP MAS connection to MSE based on the RFCOMM channel or L2CAP depends on whether the GoepL2capPsm is found in SDP record.
   If the GoepL2capPsm is found, this connection is based on L2CAP otherwise RFCOMM channel. 
9. After MAP MAS connection is established, input "map.mce.get_folder_list" to get folder listing from the current folder of MSE.
   If want to set the parameter "srmp", add the parameter "-srmp", like "map.mce.get_folder_list -srmp 3". 
   If want to add applicaton parameters, input like "map.mce.get_folder_list -mlc 10 -lso 0".
   All application parameters are optional and their specific information can be found in spec.
   It is possible that MSE does not support all parameters.
10. Input "map.mce.set_folder -name ./telecom" to set path to "telecom".
   Input "map.mce.set_folder -name ./msg" to set path to "msg".
   Input "map.mce.set_folder -name ./inbox" to set path to "inbox".
   When name is "/", go to root directory.
   When name is ".." or "../", go up one level.
   When name is "child" or "./child", go to child.
   When name is "../child", go up one level and then go to child.
11. After going to "inbox", input "map.mce.get_msg_list" to get Messages-Listing objects from the current folder of MSE.
   If getting Messages-Listing objects in the child folder, the name is mandatory otherwise excluded. The name shall not include any path information.
   If want to set the parameter "srmp", add the parameter "-srmp", like "map.mce.get_msg_list -srmp 3". 
   If want to add application parameters, input like "map.mce.get_msg_list -mlc 10 -lso 0 -fpb 20240713T000000".
   All application parameters are optional and their specific information can be found in spec.
   It is possible that MSE does not support all parameters.
12. After getting Messages-Listing objects successfully, input "map.mce.get_msg -name XXXXXXXXXXXXXXXX -a 0 -c 0" to get message content.
   The name "XXXXXXXXXXXXXXXX" is a message handle returned in the "map.mce.get_msg_list" repsonse.
   If want to set the parameter "srmp", add the parameter "-srmp", like "map.mce.get_msg -name XXXXXXXXXXXXXXXX -a 0 -c 0 -srmp 3".
   The "FractionRequest" application parameter is optional and its specific information can be found in spec.
   It is possible that MSE does not support all parameters.
13. After getting Messages-Listing objects successfully, input "map.mce.set_msg_status -name XXXXXXXXXXXXXXXX -si 0 -sv 0" to set message status to "unread".
   The name "XXXXXXXXXXXXXXXX" is a message handle returned in the "map.mce.get_msg_list" repsonse.
   The "ExtendedData" application parameter is optional and its specific information can be found in spec.
   It is possible that MSE does not support all parameters.
14. Input "map.mce.push_msg -c 0" to push a message to the current folder of MSE.
   If pushing message to a child folder, the name is mandatory otherwise excluded. The name shall not include any path information.
   Other application parameters are optional and their specific information can be found in spec.
   It is possible that MSE does not support all parameters.
15. Input "map.mce.set_ntf_reg -ns 1" to register itself for being notified of the arrival of new messages.
   After receiving this command, MSE will initiate a MAP MNS connection. After MAP MNS connection is established, MSE can send event to MCE.
16. Input "map.mce.update_inbox" to initiate an update of the MSE's inbox.
   MSE shall contact the network to retrieve new messages if available.
17. Input "map.mce.get_mas_inst_info -mii 0" to get user-readable information about the MAS Instances provided by the MSE.
   If want to set the parameter "srmp", add the parameter "-srmp", like "map.mce.get_mas_inst_info -mii 0 -srmp 3".
18. Input "map.mce.set_owner_status -cs 1" to change the Chat State of the owner on the MSE.
   At least one application parameter is selected to set.
   Other application parameters are optional and their specific information can be found in spec.
   It is possible that MSE does not support all parameters.
   The command will not be sent if the 'Owner status' bit in the MapSupportedFeatures of the MSE is not set.
19. Input "map.mce.get_owner_status -ci XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" to the Presence, Chat State, or Last Activity of the owner on MSE.
   If want to set the parameter "srmp", add the parameter "-srmp", like "map.mce.get_owner_status -ci XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX -srmp 3".
   It is possible that MSE is not able to handle the ConversationID.
   The command will not be sent if the 'Owner status' bit in the MapSupportedFeatures of the MSE is not set.
20. Input "map.mce.get_convo_list" to get Conversation-Listing objects from MSE.
   If want to set the parameter "srmp", add the parameter "-srmp", like "map.mce.get_convo_list -srmp 3".
   If want to add application parameters, input like "map.mce.get_convo_list -mlc 10 -lso 0".
   All application parameters are optional and their specific information can be found in spec.
   It is possible that MSE does not support all parameters.
   The command will not be sent if the 'Conversation listing' bit in the MapSupportedFeatures of the MSE is not set.
21. Input "map.mce.set_ntf_filter -nfm 7FFF" to unmask all notifications.
   The command will not be sent if the 'Notification Filtering' bit in the MapSupportedFeatures of the MSE is not set.
22. Input "map.mce.set_ntf_reg -ns 0" to unregister itself for being notified of the arrival of new messages.
   After receiving this command, MSE will disconnect the MAP MNS connection.
   MAP MNS connection also can be disconnected by inputting "map.mce.mns_disconnect".
23. Input "map.mce.disconnect" to disconnect MAP MAS connection to MSE
   If MSE is a mobile phone, it will disconnect ACL connect proactively after MAP MAS diconnection.

Note:
1. If using the a mobile phone as an MSE to actively connect to MCE, please be sure to follow step 5 and ignore step 6, 7.
   After testing, it was found that some mobile phones would only actively query for major services such as HFP or A2DP etc,
   and if it did not find them or establish an L2CAP connection within the allotted time, the phone would actively disconnect ACL connection.
   If the MCE initiates a connection to a mobile phone actively, step 5 is optional.
2. If MSE is a mobile iphone, after connection, user needs to actively turn on the permission to allow access to messages.
   Otherwise MCE can not get message from MSE.
3. It is possible that MSE does not support all MAP commands and application parameters.

Here is an example of run MSE, and MCE is a board running a shell project.

1. Input "bt.init" to initialize bluetooth.
2. After bluetooth init successfully, Input "br.pscan on" to turn on pscan, MSE can be discoverable.
3. Input "br.iscan on" to turn on iscan, MSE can be connectable.
4. Input "map.mse.register" to init MAP MSE MAS. Input "map.mse.mns_register" to init MAP MSE MNS.
5. After that, on MCE side, perform the above steps(step 1 - 23) to test MSE functions.
6. After MAP MNS connection is established, input "map.mse.send_event -mii 0" to send MAP-Event-Report object to MCE.
7. Input "map.mse.mns_disconnect" to disconnect MAP MNS connection.
   MAP MNS connection also can be disconnected by inputting "map.mce.set_ntf_reg -ns 0" or "map.mce.mns_disconnect" on MCE side.
8. Input "map.mse.disconnect" to disconnect MAP MAS connection.
   MAP MAS connection also can be disconnected by inputting "map.mce.disconnect" on MCE side.

Note:
1. Due to the memory size limitation, some boards don't enable MSE function, such as MIMXRT1170-EVKB.
   If users wants to enable MSE function, please set the macro of CONFIG_BT_MAP_MSE to 1 and set the macro of RAM_DISK_ENABLE to 1.
2. This example acting as MSE doesn't supports all application parameters and only supports to parse/send the part of application parameters from/to MCE.
3. This example acting as MSE is based on FatFs RAM disk. There is a limited memory to store the incoming message from MCE.
