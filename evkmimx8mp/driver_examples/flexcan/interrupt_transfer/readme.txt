Overview
========
The flexcan_interrupt example shows how to use FlexCAN driver in none-blocking interrupt way:

In this example, 2 boards are connected through CAN bus. Endpoint A(board A) send a CAN Message to
Endpoint B(board B) when user press space key in terminal. Endpoint B receive the message, print
the message content to terminal and echo back the message. Endpoint A will increase the received
message and waiting for the next transmission of the user initiated.

For self wake up from STOP mode, since steps which MCU enters STOP mode differs on different MCUs,
take flexcan_interrupt_transfer of twrke18f for example, user should do like this:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#include "fsl_smc.h"
...
...
flexcanConfig.enableSelfWakeup = true;
FLEXCAN_Init();
...
...
SMC_SetPowerModeStop(SMC, kSMC_PartialStop1);
if (wakenUp)
{
    PRINTF("B has been waken up!\r\n\r\n");
}
...
...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Toolchain supported
===================
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMX8M Plus board
- J-Link Debug Probe
- 12V power supply
- Personal Computer

Board settings
==============
The example requires 2 sets of boards, each of them are mounted with the base board. Using a male to male CAN
cable to connect the CAN1 instance (J19) between the 2 base boards.

Prepare the Demo
================
1.  Connect 12V power supply and J-Link Debug Probe to the board, switch SW3 to power on the board
2.  Connect a USB cable between the host PC and the J23 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
After connecting the two boards, these instructions display on each terminal window.
One board must be chosen as node A and the other board as node B. (Note: Node B should start first)
Data is sent continuously between the node A and the node B.

~~~~~~~~~~~~~~~~~~~~~
This message displays on the node A terminal:

********* FLEXCAN Interrupt EXAMPLE *********
    Message format: Standard (11 bit id)
    Message buffer 9 used for Rx.
    Message buffer 8 used for Tx.
    Interrupt Mode: Enabled
    Operation Mode: TX and RX --> Normal
*********************************************

Please select local node as A or B:
Note: Node B should start first.
Node:A
Press any key to trigger one-shot transmission

Rx MB ID: 0x123, Rx MB data: 0x0, Time stamp: 21747
Press any key to trigger the next transmission!

Rx MB ID: 0x123, Rx MB data: 0x1, Time stamp: 20717
Press any key to trigger the next transmission!
~~~~~~~~~~~~~~~~~~~~~

This message displays on the node B terminal:

********* FLEXCAN Interrupt EXAMPLE *********
    Message format: Standard (11 bit id)
    Message buffer 9 used for Rx.
    Message buffer 8 used for Tx.
    Interrupt Mode: Enabled
    Operation Mode: TX and RX --> Normal
*********************************************

Please select local node as A or B:
Note: Node B should start first.
Node:B
Start to Wait data from Node A

Rx MB ID: 0x321, Rx MB data: 0x0, Time stamp: 46379
Wait Node A to trigger the next transmission!

Rx MB ID: 0x321, Rx MB data: 0x1, Time stamp: 45372
Wait Node A to trigger the next transmission!
~~~~~~~~~~~~~~~~~~~~~
