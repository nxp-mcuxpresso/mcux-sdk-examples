Overview
========
The spi_interrupt_b2b_transfer example shows how to use SPI CMSIS driver in interrupt way:

In this example , we need two boards, one board used as SPI master and another board used as SPI slave.
The file 'spi_interrupt_b2b_transfer_slave.c' includes the SPI slave code.
This example uses the transactional API in SPI driver.

1. SPI master send/received data to/from SPI slave in interrupt . 

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
- FRDM-MCXC041 board
- Personal Computer

Board settings
==============
SPI one board:
  + Transfer data from MASTER_BOARD to SLAVE_BOARD of SPI interface, SPI0 pins of MASTER_BOARD are connected with
    SPI0 pins of SLAVE_BOARD
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
           MASTER_BOARD    CONNECTS TO   SLAVE_BOARD
Pin Name   Board Location                Board Location
MOSI       J2-8                          J2-8
MISO       J2-10                         J2-10
SS         J2-6                          J2-6
CLK        J2-12                         J2-12
GND        J2-14                         J2-14
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 9600 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Reset the SoC and run the project.

Running the demo
================
When the example runs successfully, the following message is displayed in the terminal:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SPI CMSIS driver board to board dma example.

 Slave example is running...


 Slave receive:
      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
     10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F
     20 21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F
     30 31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F
Succeed!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
