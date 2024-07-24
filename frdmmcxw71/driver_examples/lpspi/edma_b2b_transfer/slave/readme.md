Overview
========
The lpspi_edma_b2b_transfer example shows how to use LPSPI driver in edma way:

In this example , we need two boards, one board used as LPSPI master and another board used as LPSPI slave.
The file 'lpspi_edma_b2b_transfer_slave.c' includes the LPSPI slave code.

1. LPSPI master send/received data to/from LPSPI slave in edma . (LPSPI Slave using edma to receive/send the data)

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXW71 Board
- Personal Computer

Board settings
==============
To make the example work, connections needed to be as follows:
Note: Be sure to disconnect the 12V adapter on J9, otherwise the transmission will fail.
Jumper setting:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MASTER(SPI0)                connect to      SLAVE(SPI0)
Pin Name   Board Location                   Pin Name    Board Location
SOUT       J2 pin 9                         SIN         J1 pin 2
SIN        J1 pin 2                         SOUT        J2 pin 9
SCK        J1 pin 5                         SCK         J1 pin 5
PCS0       J1 pin 1                         PCS0        J1 pin 1
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Other jumpers keep default configuration.

Prepare the Demo
================
1. Connect a mini USB cable between the PC host and the OpenSDA USB port on the board.
2. Open a serial terminal on PC for OpenSDA serial device with these settings:
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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LPSPI board to board edma example.

 Slave example is running...
This is LPSPI slave edma transfer completed callback. 

This is LPSPI slave edma transfer completed callback. 


 Slave receive:
      1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 10
     11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20
     21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30
     31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F 40

 Slave example is running...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

