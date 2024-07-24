Overview
========
The lpspi_interrupt_b2b_transfer example shows how to use LPSPI driver in interrupt way:

In this example, we need two boards, one board used as LPSPI master and another board used as LPSPI slave.
The file 'lpspi_interrupt_b2b_transfer_master.c' includes the LPSPI master code.
This example uses the transactional API in LPSPI driver.
LPSPI master send/received data to/from LPSPI slave in interrupt. (LPSPI Slave using interrupt to receive/send the data)



SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- USB Type-C cable
- two MCIMX93-EVK boards
- J-Link Debug Probe
- 12V~20V power supply
- Personal Computer

Board settings
==============
Transfer data from one board instance to another board's instance.
SPI3 pins are connected with SPI3 pins of another board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
INSTANCE0(SPI3)     CONNECTS TO         INSTANCE0(SPI3)
Pin Name   Board Location       Pin Name  Board Location
SPI3_SCK    J1001 pin 23        SPI3_SCK  J1001 pin 23
SPI3_SIN    J1001 pin 21        SPI3_SOUT J1001 pin 19
SPI3_SOUT   J1001 pin 19        SPI3_SIN  J1001 pin 21
SPI3_PCS0   J1001 pin 24        SPI3_PCS0 J1001 pin 24
GND                             GND
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Note
Please run the application in Low Power boot mode (without Linux BSP).
The IP module resource of the application is also used by Linux BSP.
Or, run with Single Boot mode by changing Linux BSP to avoid resource
conflict.

Prepare the Demo
================
1.  Connect 12V~20V power supply and J-Link Debug Probe to the board, switch SW301 to power on the board.
2.  Connect a USB Type-C cable between the host PC and the J1401 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either cold boot your board or launch the debugger in your IDE to begin running the example.


Running the demo
================
When the demo runs successfully, the log would be seen in terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LPSPI interrupt board to board (b2b) transfer master example.
This example use one board as master and another as slave.
Master and slave uses interrupt way. Slave should start first.
Please make sure you make the correct line connection. Basically, the connection is:
LPSPI_master -- LPSPI_slave
    CLK      --    CLK
    PCS      --    PCS
    SOUT     --    SIN
    SIN      --    SOUT
    GND      --    GND

 Master transmit:
  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 10
 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20
 21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30
 31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F 40

LPSPI transfer all data matched!

 Master received:
  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 10
 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20
 21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30
 31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F 40

Press any key to run again
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

