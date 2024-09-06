Overview
========
The lpspi_interrupt_b2b_transfer example shows how to use LPSPI driver in interrupt way:

In this example , we need two boards, one board used as LPSPI master and another board used as LPSPI slave.
The file 'lpspi_interrupt_b2b_transfer_slave.c' includes the LPSPI slave code.
This example uses the transactional API in LPSPI driver.

1. LPSPI master send/received data to/from LPSPI slave in interrupt . (LPSPI Slave using interrupt to receive/send the data)

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Micro USB cable
- Two MIMXRT700-EVK boards
- Personal Computer

Board settings
==============
1. Make sure JP12 is installed
2. SPI two boards:
Transfer data from one board instance to another board's instance.
SPI14 pins are connected with SPI14 pins of another board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
INSTANCE0(SPI14)     CONNECTS TO         INSTANCE0(SPI14)
Pin Name   Board Location           Pin Name  Board Location
SOUT        J20 pin 3                   SIN       J20 pin 5
SIN         J20 pin 5                   SOUT      J20 pin 3
SCK         J20 pin 7                   SCK       J20 pin 7
PCS0        J20 pin 1                   PCS0      J20 pin 1
GND         J20 pin 9                   GND       J20 pin 9
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port on the board.
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
The following lines are printed to the serial terminal when the demo program is executed.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LPSPI board to board interrupt example.

 Slave example is running...
This is LPSPI slave transfer completed callback.
It's a successful transfer.

This is LPSPI slave transfer completed callback.
It's a successful transfer.


 Slave received:
      1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 10
     11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20
     21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30
     31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F 40

 Slave example is running...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
