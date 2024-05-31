Overview
========
The example shows how to use i3c component as bus secondary master and demo the mastership take-over process.

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.3
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXA156 board
- Personal Computer

Board settings
==============
I3C one board:
  + Transfer data from MASTER_BOARD to SLAVE_BOARD of I3C interface, I3C0 pins of MASTER_BOARD are connected with
    I3C0 pins of SLAVE_BOARD
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MASTER_BOARD    CONNECTS TO       SLAVE_BOARD
Pin Name        Board Location    Pin Name   Board Location
I3C0_SCL/P1_9   J8-3              P1_9       J8-3
I3C0_SDA/P1_8   J8-4              P1_8       J8-4
GND             J5-8              GND        J5-8
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1.  Connect a USB Type-C cable between the host PC and the MCU-Link USB port on the target board.
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
