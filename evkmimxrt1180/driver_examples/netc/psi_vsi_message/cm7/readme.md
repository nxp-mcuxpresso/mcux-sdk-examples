Overview
========
The Multicore NETC PSI-VSI demo application demonstrates how to set up projects for individual
cores on a multicore system. In this demo, the primary core use NETC PSI to send/receive frames.
It also releases the secondary core from the reset. The secondary core toggles an on-board LED
indicating that the secondary core is running and uses NETC VSI to send/receive frames.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1180-EVK board
- Personal Computer

Board settings
==============
Populate R469 and set reset pin as open-drain to complete the PHY hardware reset properly.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs successfully, the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
NETC PSI VSI transfer from the Primary Core!

Copy Secondary core image to address: 0x303c0000, size: 22295

NETC Switch frame loopback example start.
Wait for PHY link up...
Starting Secondary core.
The secondary core application has been started.
VSI EP initialization succeeded.
 PSI sends message 0 to VSI
 PSI receives message from VSI to set PSI MAC address 54:27:8d:0:0:0.
 A frame received from VSI. The length is 1000. Dest Address ff:ff:ff:ff:ff:ff Src Address 54:27:8d:aa:bb:cc.
 A frame received from PSI. The length is 1000. Dest Address ff:ff:ff:ff:ff:ff Src Address 54:27:8d: 0: 0: 0.
 PSI sends message 1 to VSI
 PSI receives message from VSI to set PSI MAC address 54:27:8d:0:0:1.
 A frame received from VSI. The length is 1000. Dest Address ff:ff:ff:ff:ff:ff Src Address 54:27:8d:aa:bb:cc.
 A frame received from PSI. The length is 1000. Dest Address ff:ff:ff:ff:ff:ff Src Address 54:27:8d: 0: 0: 1.
 PSI sends message 2 to VSI
 PSI receives message from VSI to set PSI MAC address 54:27:8d:0:0:2.
 A frame received from VSI. The length is 1000. Dest Address ff:ff:ff:ff:ff:ff Src Address 54:27:8d:aa:bb:cc.
 A frame received from PSI. The length is 1000. Dest Address ff:ff:ff:ff:ff:ff Src Address 54:27:8d: 0: 0: 2.

......

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
