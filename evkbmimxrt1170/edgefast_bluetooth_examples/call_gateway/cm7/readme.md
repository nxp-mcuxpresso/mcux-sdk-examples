Overview
========
Application demonstrating how to use the telephone call gateway feature of LE Audio.


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

Murata Solution Board settings
Embedded Artists M.2 module resource page: https://www.embeddedartists.com/m2
Embedded Artists 2EL module datasheet: https://www.embeddedartists.com/doc/ds/2EL_M2_Datasheet.pdf

The hardware rework for MIMXRT1170-EVKB and Murata 2EL M.2 Adapter is same as MIMXRT1170-EVKB and Murata 1XK M.2 Adapter.
Note:
After downloaded binary into qspiflash and boot from qspiflash directly,
please reset the board by pressing SW4 or power off and on the board to run the application.
Prepare the Demo
================

1.  Open example's project and build it.

2.  Connect a USB cable between the PC host and the OpenSDA USB port on the target board.

3.  Provide 5V voltage for the target board.

4.  Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control

5.  Download the program to the target board.

6.  Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Note: the example can work with call_terminal example. Please refer to the readme of call_terminal to prepare the call_terminal exmaple.

Running the demo
================
The log below shows the output of the example in the terminal window.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Copyright  2022  NXP

call_gateway>> Bluetooth initialized
Get required Source Capability from codec. Codec configurations:
    Frequency 16000
    Duration 10000
    Frame bytes 40
    Frame blocks per SDU 1
    Location 3, channel count 2.
Get required Sink Capability from codec. Codec configurations:
    Frequency 16000
    Duration 10000
    Frame bytes 40
    Frame blocks per SDU 1
    Location 3, channel count 2.
Scanning started

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Procedures to run
1 input "help" to show command list,
2 the scanning of the device is started automatically. It starts to scann the call_terminal device,
3 After the conenction is estabilished, the log is following.
Please note that if the Security is changed with error 0x02, it means the key is missing. Please clear bonding information by sending
commander "unpair". And, retry again.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Found device: A0:CD:F3:77:E6:15 (public)MTU exchanged: 23/23
Connected to peer: A0:CD:F3:77:E6:15 (public)
MTU exchanged: 65/65
Security changed: A0:CD:F3:77:E6:15 (public) level 2 (error 0)
codec capabilities on conn 202DE3C0 dir 1 codec 2000D054. Codec configurations:
    Frequency 8000, 16000, 24000, 32000, 44100, 48000,
    Duration 10000,
    Channel count 2.
    Frame length min 40, max 120
    Frame blocks per SDU 1
    Pref context 0x206
conn 202DE3C0 dir 1 loc 3
conn 202DE3C0 snk ctx 519 src ctx 3
conn 202DE3C0 dir 1 ep 202DAC30
Discover sinks complete: err 0
codec capabilities on conn 202DE3C0 dir 2 codec 2000D054. Codec configurations:
    Frequency 8000, 16000, 24000, 32000, 44100, 48000,
    Duration 10000,
    Channel count 2.
    Frame length min 40, max 120
    Frame blocks per SDU 1
    Pref context 0x206
conn 202DE3C0 dir 2 loc 3
conn 202DE3C0 snk ctx 519 src ctx 3
conn 202DE3C0 dir 2 ep 202DAE38
Discover sources complete: err 0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
4 After the message "Discover complete (err 0)! TBS count 1, GTBS found? Yes" is printed on call_terminal side. All feature are ready.
5.1. Start the call by local. Enter "call_outgoing 0 <XX>:<YY>" on the call_gateway side, or enter "call_outgoing 0 <XX>:<YY>" on the call_terminal side,
The log is following,
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Start a outgoing call, call index 1, callee tel:qq
Audio Stream 202F0688 configured
Audio Stream 202F0650 configured
Audio Stream 202F0688 QoS set
Audio Stream 202F0650 QoS set
Audio Stream 202F0688 enabled
Init Audio SAI and CODEC, samplingRate :16000  bitWidth:16
Set default headphone volume 70
Audio Stream 202F0650 enabled
Audio Stream 202F0650 started
Audio Stream 202F0688 started
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
5.2 Start a call by remote. Enter "remote_call_incoming 0 <AA>:<BB> <CC>:<DD> <EE>" on the call_gateway side.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
remote_call_incoming 0 tel:qq tel:qq qq
incoming call: callee uri tel:qq, caller uri tel:qq
Audio Stream 202F0688 configured
Audio Stream 202F0650 configured
Audio Stream 202F0688 QoS set
Audio Stream 202F0650 QoS set
Audio Stream 202F0688 enabled
Init Audio SAI and CODEC, samplingRate :16000  bitWidth:16
Set default headphone volume 70
Audio Stream 202F0650 enabled
Audio Stream 202F0650 started
Audio Stream 202F0688 started
done, call index is 0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

6.1 Reject/end the remote call, enter "call_term <call_index>" on the call_gateway side
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
terminate the call: call index 1
Audio Stream 202F0688 disabled
Audio Stream 202F0688 QoS set
Audio Stream 202F0650 disabled
Fail to stop stream (err -77)
Audio Stream 202F0650 QoS set
Audio Stream 202F0688 stopped with reason 0x13
Audio Stream 202F0650 stopped with reason 0x13
Audio Stream 202F0688 released
Audio Stream 202F0650 released
Return code 0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Or, enter "call_term 0 <call_index>" on the call_terminal side
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Terminate a call, call index 1, reason 6
Audio Stream 202F0688 disabled
Audio Stream 202F0688 QoS set
Audio Stream 202F0650 disabled
Fail to stop stream (err -77)
Audio Stream 202F0650 QoS set
Audio Stream 202F0688 stopped with reason 0x13
Audio Stream 202F0650 stopped with reason 0x13
Audio Stream 202F0688 released
Audio Stream 202F0650 released
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
6.2 Reject/end the call by remote. enter "remote_call_term <call_index>" on the call_gateway side
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Remove terminate the call: call index 1
Audio Stream 202F0688 disabled
Audio Stream 202F0688 QoS set
Audio Stream 202F0650 disabled
Fail to stop stream (err -77)
Audio Stream 202F0650 QoS set
Audio Stream 202F0688 stopped with reason 0x13
Audio Stream 202F0650 stopped with reason 0x13
Audio Stream 202F0688 released
Audio Stream 202F0650 released
Return code 0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

7.1 Accept the remote call. enter "call_accept <call_index>" on the call_gateway side
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
accept the call: call index 1
Audio Stream 202F0688 disabled
Audio Stream 202F0688 QoS set
Audio Stream 202F0650 disabled
Fail to stop stream (err -77)
Audio Stream 202F0650 QoS set
Audio Stream 202F0688 stopped with reason 0x13
Audio Stream 202F0650 stopped with reason 0x13
Audio Stream 202F0688 enabled
Init Audio SAI and CODEC, samplingRate :16000  bitWidth:16
Set default headphone volume 70
Audio Stream 202F0650 enabled
Audio Stream 202F0650 started
Audio Stream 202F0688 started
Return code 0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Or, enter "call_accept 0 <call_index>" on the call_terminal side
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Audio Stream 202F0688 disabled
Audio Stream 202F0688 QoS set
Audio Stream 202F0650 disabled
Fail to stop stream (err -77)
Audio Stream 202F0650 QoS set
Audio Stream 202F0688 stopped with reason 0x13
Audio Stream 202F0650 stopped with reason 0x13
Audio Stream 202F0688 enabled
Init Audio SAI and CODEC, samplingRate :16000  bitWidth:16
Set default headphone volume 70
Audio Stream 202F0650 enabled
Audio Stream 202F0650 started
Audio Stream 202F0688 started
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
7.2 Accept the call by remote device. enter "remote_call_answer <call_index>" on the call_gateway side
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Remove answer the call: call index 1
Audio Stream 202F0688 disabled
Audio Stream 202F0688 QoS set
Audio Stream 202F0650 disabled
Fail to stop stream (err -77)
Audio Stream 202F0650 QoS set
Audio Stream 202F0688 stopped with reason 0x13
Audio Stream 202F0650 stopped with reason 0x13
Audio Stream 202F0688 enabled
Init Audio SAI and CODEC, samplingRate :16000  bitWidth:16
Set default headphone volume 70
Audio Stream 202F0650 enabled
Audio Stream 202F0650 started
Audio Stream 202F0688 started
Return code 0
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


