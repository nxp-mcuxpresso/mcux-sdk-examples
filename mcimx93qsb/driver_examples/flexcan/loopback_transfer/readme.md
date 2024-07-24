Overview
========
The flexcan_loopback example shows how to use the loopback test mode to debug your CAN Bus design:

To demonstrates this example, only one board is needed. The example will config one FlexCAN Message
Buffer to Rx Message Buffer and the other FlexCAN Message Buffer to Tx Message Buffer with same ID.
After that, the example will send a CAN Message from the Tx Message Buffer to the Rx Message Buffer
through internal loopback interconnect and print out the Message payload to terminal.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- USB Type-C cable
- MCIMX93-QSB board
- JLink Plus
- 12V~20V power supply
- Personal Computer

Board settings
==============
No special is needed.

Prepare the Demo
================
1.  Connect 12V~20V power supply and JLink Plus to the board, switch SW301 to power on the board
2.  Connect a USB Type-C cable between the host PC and the J1708 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either re-power up your board or launch the debugger in your IDE to begin running the demo.

Note
Please run the application in Low Power boot mode (without Linux BSP).
The IP module resource of the application is also used by Linux BSP.
Or, run with Single Boot mode by changing Linux BSP to avoid resource
conflict.

Running the demo
================
When the example runs successfully, following information can be seen on the terminal:

~~~~~~~~~~~~~~~~~~~~~

==FlexCAN loopback functional example -- Start.==


Send message from MB0 to MB1

tx word0 = 0x11223344

tx word1 = 0x55667788


Receved message from MB1

rx word0 = 0x11223344

rx word1 = 0x55667788


==FlexCAN loopback functional example -- Finish.==
~~~~~~~~~~~~~~~~~~~~~
