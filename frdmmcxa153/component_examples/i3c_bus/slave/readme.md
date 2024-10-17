Overview
========
The example shows how to use i3c component as bus secondary master and demo the mastership take-over process.

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
- FRDM-MCXA153 board
- Personal Computer

Board settings
==============
I3C one board:
  + Transfer data from MASTER_BOARD to SLAVE_BOARD of I3C interface, I3C0 pins of MASTER_BOARD are connected with
    I3C0 pins of SLAVE_BOARD
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MASTER_BOARD        CONNECTS TO         SLAVE_BOARD
Pin Name   Board Location     Pin Name   Board Location
P0_16      J20                P0_16      J20
P0_17      J21                P0_17      J21
GND        J6-8               GND        J6-8
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1.  Connect a Type-C USB cable between the PC host and the CMSIS DAP USB port (J15) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Press reset button to begin running the demo.

Running the demo
================
The following message shows in the terminal if the example runs successfully.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~
I3C bus slave example.
I3C bus secondary master creates.
I3C bus second master requires to be primary master.
I3C bus mastership takeover.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
