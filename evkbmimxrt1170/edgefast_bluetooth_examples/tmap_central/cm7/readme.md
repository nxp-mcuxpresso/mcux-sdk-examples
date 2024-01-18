Overview
========
Application demonstrating how to use the tmap central feature.


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
- evkbmimxrt1170 board
- Personal Computer
- Embedded Artists 2EL M.2 Module (Rev-A1) - direct M2 connection.

Jumper settings for RT1170-EVKB (enables external 5V supply):
remove  J38 5-6
connect J38 1-2
connect J43 with external power(controlled by SW5)

Murata Solution Board settings
Embedded Artists 2EL module datasheet: https://www.embeddedartists.com/doc/ds/2EL_M2_Datasheet.pdf

The hardware should be reworked according to the Hardware Rework Guide for MIMXRT1170-EVKB and Murata 1XK M.2 Adapter in document Hardware Rework Guide for EdgeFast BT PAL.
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
CAP initialized
VCP initialized
MCP initialized
CCP initialized
Scanning successfully started
[DEVICE]: 41:77:CE:FE:99:1B (random), [AD]: 1 data_len 1
[AD]: 255 data_len 22
[AD]: 3 data_len 2
[DEVICE]: 56:CF:9F:4C:98:40 (random), [AD]: 1 data_len 1
[AD]: 25 data_len 2
[AD]: 2 data_len 2
[AD]: 46 data_len 6
[AD]: 22 data_len 4
Found TMAS in peer adv data!
Attempt to connect!
MTU exchanged: 23/23
Connected: 56:CF:9F:4C:98:40 (random)
MTU exchanged: 65/65
Security changed: 0
TMAS discovery done
Error sending mute command!
Found CAS
codec_capabilities 202CE99C dir 0x01
codec 0x06 cid 0x0000 vid 0x0000 count 4
data #0: type 0x01 len 1

data #1: type 0x02 len 1

data #2: type 0x03 len 4
010000
data #3: type 0x04 len 2
1e
meta #0: type 0x02 len 2
07
codec_capabilities 202CE99C dir 0x01
codec 0x06 cid 0x0000 vid 0x0000 count 4
data #0: type 0x01 len 1

data #1: type 0x02 len 1

data #2: type 0x03 len 4
010000
data #3: type 0x04 len 2
3c
meta #0: type 0x02 len 2
07
codec_capabilities 202CE99C dir 0x01
codec 0x06 cid 0x0000 vid 0x0000 count 4
data #0: type 0x01 len 1

data #1: type 0x02 len 1

data #2: type 0x03 len 4
010000
data #3: type 0x04 len 2
50
meta #0: type 0x02 len 2
07
codec_capabilities 202CE99C dir 0x01
codec 0x06 cid 0x0000 vid 0x0000 count 4
data #0: type 0x01 len 1

data #1: type 0x02 len 1

data #2: type 0x03 len 4
010000
data #3: type 0x04 len 2
4b
meta #0: type 0x02 len 2
07
codec_capabilities 202CE99C dir 0x01
codec 0x06 cid 0x0000 vid 0x0000 count 4
data #0: type 0x01 len 1

data #1: type 0x02 len 1

data #2: type 0x03 len 4
010000
data #3: type 0x04 len 2
64
meta #0: type 0x02 len 2
07
codec_capabilities 202CE99C dir 0x01
codec 0x06 cid 0x0000 vid 0x0000 count 4
data #0: type 0x01 len 1

data #1: type 0x02 len 1

data #2: type 0x03 len 4
010000
data #3: type 0x04 len 2
5a
meta #0: type 0x02 len 2
07
codec_capabilities 202CE99C dir 0x01
codec 0x06 cid 0x0000 vid 0x0000 count 4
data #0: type 0x01 len 1

data #1: type 0x02 len 1

data #2: type 0x03 len 4
010000
data #3: type 0x04 len 2
78
meta #0: type 0x02 len 2
07
codec_capabilities 202CE99C dir 0x01
codec 0x06 cid 0x0000 vid 0x0000 count 4
data #0: type 0x01 len 1

data #1: type 0x02 len 1

data #2: type 0x03 len 4
010000
data #3: type 0x04 len 2
75
meta #0: type 0x02 len 2
07
codec_capabilities 202CE99C dir 0x01
codec 0x06 cid 0x0000 vid 0x0000 count 4
data #0: type 0x01 len 1

data #1: type 0x02 len 1

data #2: type 0x03 len 4
010000
data #3: type 0x04 len 2
9b
meta #0: type 0x02 len 2
07
Sink #0: ep 202C2F44
Sink discover complete
Discover sources complete: err 0
Created group
Configured stream 202C83DC
QoS set stream 202C83DC
Enabled stream 202C83DC
Started stream 202C83DC
Sending mock data with len 100
CCP: Placing call to remote with id 1 to skype:friend
Sending mock data with len 100
Sending mock data with len 100
CCP: Call terminated for id 1 with reason 6
Sending mock data with len 100
Sending mock data with len 100
Sending mock data with len 100
Sending mock data with len 100
Sending mock data with len 100
Sending mock data with len 100
Sending mock data with len 100
Sending mock data with len 100
Sending mock data with len 100
Sending mock data with len 100
Sending mock data with len 100
Sending mock data with len 100
Sending mock data with len 100
Sending mock data with len 100
Sending mock data with len 100
Sending mock data with len 100
Sending mock data with len 100
Sending mock data with len 100
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Procedures to run
1 this demo don't need any user input and will run automatically.
