Overview
========
The lpspi_interrupt_b2b example shows how to use LPSPI driver in interrupt way:

In this example , we need two boards , one board used as LPSPI master and another board used as LPSPI slave.
The file 'lpspi_interrupt_b2b_master.c' includes the LPSPI master code.
This example does not use the transactional API in LPSPI driver. It's a demonstration that how to use the interrupt in KSDK driver.

1. LPSPI master send/received data to/from LPSPI slave in interrupt . (LPSPI Slave using interrupt to receive/send the data)

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

LPSPI:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
       MASTER(SPI0)      connect to        SLAVE(SPI1)
Pin Name   Board Location     Pin Name    Board Location
SOUT       J6-6                SIN       J3-9
SIN        J6-5                SOUT      J3-11
SCK        J6-4                SCK       J3-7
PCS0       J6-3                PCS1      J3-5
GND        J6-8                GND       J6-8
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1. Connect a USB Type-C cable between the host PC and the MCU-Link USB port on the target board.
2. Open a serial terminal on PC for MCU-Link serial device with these settings:
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
The following lines are printed to the serial terminal when the demo program is executed.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
lpspi_functional_interrupt_board_2_board_master start.
This example use one board as master and the other as slave.
Please make sure you make the correct line connection. Basically, the connection is:
LPSPI_master -- LPSPI_slave
   CLK      --    CLK
   PCS      --    PCS
   SOUT     --    SIN
   SIN      --    SOUT
   GND      --    GND
Please running slave here, then type any key to continue

LPSPI transfer all data matched!

 Master received:

  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F
 20 21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F
 30 31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F
End of master example!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

