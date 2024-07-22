Overview
========
The flexcan_pingpang_buffer_transfer example shows how to use the FlexCAN queue feature to create
2 simulate FIFOs that can receive CAN/CANFD frames:

In this example, 2 boards are connected through CAN bus. Endpoint A(board A) send CAN/CANFD messages to
Endpoint B(board B) when user inputs the number of CAN messages to be sent in terminal. Endpoint B uses
two receiving queues to receive messages in turn, and prints the message content and the receiving queue
number to the terminal after any queue is full.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- USB Type-C cable
- Two MCIMX93-QSB board
- JLink Plus
- 12V~20V power supply
- Personal Computer

Board settings
==============
Switch S1501 to "ON" for two boards
The example requires connecting between CAN pins of two boards.
The connection should be set as follows:

Between two boards:
- J1501-2(CANH) node A, J1501-2(CANH) node B
- J1501-3(CANL) node A, J1501-3(CANL) node B
- J1406-7(GND) node A, J1406-7(GND) node B

Note
Please run the application in Low Power boot mode (without Linux BSP).
The IP module resource of the application is also used by Linux BSP.
Or, run with Single Boot mode by changing Linux BSP to avoid resource
conflict.

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

Running the demo
================
After connecting the two boards, these instructions display on each terminal window.
One board must be chosen as node A and the other board as node B. (Note: Node B should start first)
Data is sent continuously between the node A and the node B.

~~~~~~~~~~~~~~~~~~~~~
This message displays on the node A terminal:

********* FLEXCAN PingPong Buffer Example *********
    Message format: Standard (11 bit id)
    Node B Message buffer 1 to 4 used as Rx queue 1.
    Node B Message buffer 5 to 8 used as Rx queue 2.
    Node A Message buffer 8 used as Tx.
*********************************************

Please select local node as A or B:
Note: Node B should start first.
Node:A
Please input the number of CAN/CANFD messages to be send and end with enter.
100
Transmission done.

Please input the number of CAN/CANFD messages to be send and end with enter.
~~~~~~~~~~~~~~~~~~~~~

This message displays on the node B terminal:

This message displays on the node B terminal:

********* FLEXCAN PingPong Buffer Example *********
    Message format: Standard (11 bit id)
    Node B Message buffer 1 to 4 used as Rx queue 1.
    Node B Message buffer 5 to 8 used as Rx queue 2.
    Node A Message buffer 8 used as Tx.
*********************************************

Please select local node as A or B:
Note: Node B should start first.
Node:B
Start to Wait data from Node A

Read Rx MB from Queue 1.
Rx MB ID: 0x321, Rx MB data: 0x0, Time stamp: 20971
Rx MB ID: 0x321, Rx MB data: 0x1, Time stamp: 56187
Rx MB ID: 0x321, Rx MB data: 0x2, Time stamp: 56867
Rx MB ID: 0x321, Rx MB data: 0x3, Time stamp: 57547
Read Rx MB from Queue 2.
Rx MB ID: 0x321, Rx MB data: 0x4, Time stamp: 56187
Rx MB ID: 0x321, Rx MB data: 0x5, Time stamp: 56867
Rx MB ID: 0x321, Rx MB data: 0x6, Time stamp: 57547
Rx MB ID: 0x321, Rx MB data: 0x7, Time stamp: 57547
Wait Node A to trigger the next 8 messages!

Read Rx MB from Queue 1.
Rx MB ID: 0x321, Rx MB data: 0x8, Time stamp: 61657
Rx MB ID: 0x321, Rx MB data: 0x9, Time stamp: 31304
Rx MB ID: 0x321, Rx MB data: 0xa, Time stamp: 31983
Rx MB ID: 0x321, Rx MB data: 0xb, Time stamp: 32662
Read Rx MB from Queue 2.
Rx MB ID: 0x321, Rx MB data: 0xc, Time stamp: 31304
Rx MB ID: 0x321, Rx MB data: 0xd, Time stamp: 31983
Rx MB ID: 0x321, Rx MB data: 0xe, Time stamp: 32662
Rx MB ID: 0x321, Rx MB data: 0xf, Time stamp: 32662
Wait Node A to trigger the next 8 messages!

.....
~~~~~~~~~~~~~~~~~~~~~
