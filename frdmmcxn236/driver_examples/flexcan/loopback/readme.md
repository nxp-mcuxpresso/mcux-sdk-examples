Overview
========
The flexcan_loopback_functional example shows how to use the loopback test mode to debug your CAN Bus design:

To demonstrates this example, only one board is needed. The example will config one FlexCAN Message
Buffer to Rx Message Buffer and the other FlexCAN Message Buffer to Tx Message Buffer with same ID.
After that, the example will send a CAN Message from the Tx Message Buffer to the Rx Message Buffer
throuth internal loopback interconnect and print out the Message payload to terminal.

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
- FRDM-MCXN236 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1. Connect a USB cable between the PC host and the MCU-Link USB on the board.
2. Open a serial terminal on PC for MCU-Link serial device with these settings:
   - 115200 baud rate
   - 8 data bits
   - No parity
   - One stop bit
   - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running
   the example.

Running the demo
================
When the example runs successfully, following information can be seen on the MCU-Link terminal:

~~~~~~~~~~~~~~~~~~~~~~

==FlexCAN loopback functional example -- Start.==

Send message from MB8 to MB9
tx word0 = 0x11223344
tx word1 = 0x55667788

Received message from MB9
rx word0 = 0x11223344
rx word1 = 0x55667788

==FlexCAN loopback functional example -- Finish.==
~~~~~~~~~~~~~~~~~~~~~~
