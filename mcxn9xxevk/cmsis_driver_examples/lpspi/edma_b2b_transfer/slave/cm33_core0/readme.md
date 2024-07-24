Overview
========
The cmsis_lpspi_edma_b2b_transfer example shows how to use LPSPI CMSIS driver in edma way:

In this example , we need two boards, one board used as LPSPI master and another board used as LPSPI slave.
The file 'cmsis_lpspi_edma_b2b_transfer_slave.c' includes the LPSPI slave code.

1. LPSPI master send/received data to/from LPSPI slave in edma . 

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini USB cable
- Two MCX-N9XX-EVK boards
- Personal Computer

Board settings
==============

LPSPI:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	MASTER           connect to           SLAVE
Pin Name   Board Location     Pin Name    Board Location
SOUT       J2 Pin 8           SIN         J2 Pin 10
SIN        J2 Pin 10          SOUT        J2 Pin 8
SCK        J2 Pin 12          SCK         J2 Pin 12
PCS0       J2 Pin 6           PCS0        J2 Pin 6
GND        J2 pin 14          GND         J2 pin 14
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1.  Connect a mini USB cable between the PC host and the MCU-Link USB port on the board.
2.  Open a serial terminal on PC for MCU-Link serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Reset the SoC and run the project.

Running the demo
================
When the demo runs successfully, the log would be seen on the MCU-Link terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LPSPI CMSIS driver board to board edma example.

 Slave example is running...
This is LPSPI_SlaveSignalEvent_t
Slave receive data from master has completed!
This is LPSPI_SlaveSignalEvent_t
Slave transmit data to master has completed!

 Slave receive:
      1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 10
     11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20
     21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30
     31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F 40

 Slave example is running...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
