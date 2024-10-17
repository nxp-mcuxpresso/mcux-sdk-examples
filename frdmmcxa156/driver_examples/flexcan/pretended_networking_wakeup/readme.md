Overview
========
The flexcan_pretended_networking_wakeup example shows how to wake up FLEXCAN module from Pretended Networking mode:

In this example, 2 boards are connected through CAN bus. Endpoint B need enter STOP mode first, then endpoint A(board A) send CAN Message to Endpoint B(board B)
when user press space key in terminal. Endpoint B will wake up from STOP mode after receive 4 specific wake up frame, and print
the message content to terminal.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXA156 Board
- Personal Computer

Board settings
==============
The example requires 2 sets of boards, each of them is mounted with the base board.
To make example work, connections needed to be as follows on each base board:

Board to board connections needed to be as follows:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        Node A           connect to      Node B
Pin Name   Board Location         Pin Name    Board Location
CANH       J22-2                  CANH        J22-2
CANL       J22-4                  CANL        J22-4
GND        J22-3                  GND         J22-3
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1. Connect the type-C USB cable between the PC host and the USB ports on the board.
2. Open a serial terminal on PC for the serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running
   the demo.

Running the demo
================
After connecting the two boards, these instructions display on each terminal window.
One board must be chosen as node A and the other board as node B. (Note: Node B should start first)
Data is sent continuously between the node A and the node B.

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
