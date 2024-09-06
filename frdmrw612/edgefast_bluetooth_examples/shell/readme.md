Overview
========
Application demonstrating the shell mode of the simplified Adapter APIs.


SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Type-C cable
- frdmrw612 board
- Personal Computer

Board settings
==============

Prepare BLE controller
Download BLE controller FW according to the document component\conn_fwloader\readme.txt.

Note:
To ensure that the LITTLEFS flash region has been cleaned,
all flash sectors need to be erased before downloading example code.
After downloaded binary to the target board, 
please reset the board by pressing SW1 or power off and on the board to run the application.
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

Note: The shell information "SHELL build: Aug 10 2021" may be different, which depends on the compile date.

