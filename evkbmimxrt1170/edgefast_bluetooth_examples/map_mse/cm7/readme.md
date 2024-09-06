Overview
========
This example demonstrates the MAP MSE basic functionality, the MSE device support be connected to a MAP MCE like a Hands-Free unit in the car or a 
board running a MAP MCE application. And the MSE example support sending response/event to the MCE. 



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

Running the demo
================
The log below shows the output of the example in the terminal window.
The demo start waiting for the MCE to connect.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Bluetooth MAP MSE demo start...
Bluetooth initialized
BR/EDR set connectable and discoverable done
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Procedures to run
1. Prepare a MCE, connect the MSE device and then create MAS OBEX connection.
2. After that, MCE can send commands to MSE and MSE will respond to the commands.

The bellow commands have been supported:
1. get folder listing
    - This example will send Folder-listing object but not parse/send application parameters from/to MCE.
2. set folder
    - This example will set folder correctly.
3. get message listing
    - This example will send Messages-listing object with NewMessage, MSETime and ListingSize but not parse application parameters from MCE.
4. get message
    - This example will send bMessage object but not parse/send application parameters from/to MCE.
5. set message status
    - This example will set the read status and the deleted status correctly and save the extended data to the local buffer.
6. push message
    - This example will save the message and return a message handle but not parse application parameters from MCE.
7. set notification registration
    - When NotificationStatus is ON, this example will initiates a MNS OBEX connection.
8. update inbox
    - This example always send success when receiving update inbox request.
9. get mas instance information
    - This example will send MASInstanceInformation but not send application parameters to MCE.
10. set owner status
    - This example will save the application parameters to the local buffer that is used to respond to get owner status.
11. get owner status
    - This example will respond to get owner status with the application parameters saved in set owner status.
12. get conversation listing
    - This example will send Conversation-Listing object but not parse/send application parameters from/to MCE.
13. set notification filter
    - This example always send success when receiving set notification filter request.

Note:
This example only supports one MAS and MNS OBEX connection.
This example doesn't supports all application parameters and only supports to parse/send the part of application parameters from/to MCE.
This example is based on FatFs RAM disk. There is a limited memory to store the incoming message from MCE.

