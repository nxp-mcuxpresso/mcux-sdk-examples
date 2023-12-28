Overview
========
The flexcan_pretended_networking_wakeup example shows how to wake up FLEXCAN module from Pretended Networking mode:

In this example, 2 boards are connected through CAN bus. Endpoint B need enter STOP mode first, then endpoint A(board A) send CAN Message to Endpoint B(board B)
when user press space key in terminal. Endpoint B will wake up from STOP mode after receive 4 specific wake up frame, and print
the message content to terminal.

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- two MIMX8ULP-EVK/EVK9 board
- JLink Plus
- 5V power supply
- Personal Computer

Board settings
==============
populate J8 for two boards.
The example requires connecting between CAN pins of two boards.
The connection should be set as follows:

Between two boards:
- J8-1(CANH) node A, J8-1(CANH) node B
- J8-2(CANL) node A, J8-2(CANL) node B
- J8-3(GND) node A, J8-3(GND) node B


Prepare the Demo
================
1.  Connect 5V power supply and JLink Plus to the board, switch SW10 to power on the board
2.  Connect a micro USB cable between the host PC and the J17 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs successfully, following information can be seen on the terminal:

~~~~~~~~~~~~~~~~~~~~~
This message displays on the node A terminal:

FlexCAN pretended networking wake up example.
Please select local node as A or B:
Note: Node B should start first.
Node:a
Press any key to trigger one-shot transmission
Send message ID: 0x123, payload: 0x0055000000000000

Press any key to trigger the next transmission!
Send message ID: 0x123, payload: 0x0155000000000000

Press any key to trigger the next transmission!
Send message ID: 0x125, payload: 0x0255000000000000

Press any key to trigger the next transmission!
Send message ID: 0x125, payload: 0x0355000000000000

Press any key to trigger the next transmission!
Send message ID: 0x123, payload: 0x0455000000000000

Press any key to trigger the next transmission!
Send message ID: 0x123, payload: 0x0056000000000000

...

~~~~~~~~~~~~~~~~~~~~~

This message displays on the node B terminal:

FlexCAN pretended networking wake up example.
Please select local node as A or B:
Note: Node B should start first.
Node:b
Note B will enter lower power mode and wake up until received 4 specific messages.
Wake up message format: Standard (11 bit id)
Wake up message ID range: 0x123 to 0x124
Wake up payload range : 0x0055000000000000 to 0x0056000000000000
Waken up!
Match message 0 ID: 0x123, payload: 0x0055000000000000
Match message 1 ID: 0x123, payload: 0x0155000000000000
Match message 2 ID: 0x123, payload: 0x0455000000000000
Match message 3 ID: 0x123, payload: 0x0056000000000000
Enter lower power mode again!

~~~~~~~~~~~~~~~~~~~~~
