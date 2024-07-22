Overview
========
Application demonstrating how to use the tmap peripheral feature.


SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

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

4.  Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control

5.  Download the program to the target board.

6.  Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
The log below shows the output of the example in the terminal window.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Bluetooth initialized
Initializing TMAP and setting role
VCP initialized
BAP initialized
Advertising successfully started
Connected: A0:CD:F3:77:E4:11 (public)
Security changed: 0
TMAP discovery done
CCP: Discovered GTBS
CCP: Discovered remote URI: skype
CCP initialized
ASE Codec Config: conn 202C37D8 ep 202C1D5C dir 1
codec 0x06 cid 0x0000 vid 0x0000 count 4
data #0: type 0x01 len 1

data #1: type 0x02 len 1

data #2: type 0x03 len 4
010000
data #3: type 0x04 len 2
64
  Frequency: 48000 Hz
  Frame Duration: 10000 us
  Channel allocation: 0x1
  Octets per frame: 100 (negative means value not pressent)
  Frames per SDU: 1
ASE Codec Config stream 202C208C
QoS: stream 202C208C qos 202C1DCC
QoS: interval 10000 framing 0x00 phy 0x02 sdu 100 rtn 5 latency 20 pd 40000
Enable: stream 202C208C meta_count 1
MCP: Discovered MCS
MCP initialized
Incoming audio on stream 202C208C len 100
CCP: Call originate successful
MCP: Successfully sent command (0) - opcode: 1, param: 0
Incoming audio on stream 202C208C len 100
Incoming audio on stream 202C208C len 100
CCP: Call with id 1 terminated
MCP: Successfully sent command (0) - opcode: 2, param: 0
Incoming audio on stream 202C208C len 100
Incoming audio on stream 202C208C len 100
Incoming audio on stream 202C208C len 100
Incoming audio on stream 202C208C len 100
Incoming audio on stream 202C208C len 100
Incoming audio on stream 202C208C len 100
Incoming audio on stream 202C208C len 100
Incoming audio on stream 202C208C len 100
Incoming audio on stream 202C208C len 100
Incoming audio on stream 202C208C len 100
Incoming audio on stream 202C208C len 100
Incoming audio on stream 202C208C len 100
Incoming audio on stream 202C208C len 100
Incoming audio on stream 202C208C len 100
Incoming audio on stream 202C208C len 100
Incoming audio on stream 202C208C len 100
Incoming audio on stream 202C208C len 100
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Procedures to run
1 this demo don't need any user input and will run automatically.
