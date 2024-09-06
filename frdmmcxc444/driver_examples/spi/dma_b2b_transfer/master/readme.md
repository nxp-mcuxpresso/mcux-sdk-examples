Overview
========
The spi_dma_board2board_master example shows how to use spi driver as master to do board to board transfer with DMA:

In this example, one spi instance as master and another spi instance on othereboard as slave. Master sends a piece of
data to slave, and receive a piece of data from slave. This example checks if the data received from slave is correct.

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
- Two FRDM-MCXC444 boards
- Personal Computer

Board settings
==============
SPI board to board:
Transfers data through SPI interface. SPI0 pins of the master board are
connected to the SPI1 pins of the slave board.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MASTER BOARD           CONNECTS TO         SLAVE BOARD
Pin Name   Board Location     Pin Name  Board Location
MISO       J1 pin 11          MISO      J2 pin 10
MOSI       J1 pin 9           MOSI      J2 pin 8
SCK        J1 pin 15          SCK       J2 pin 12
PCS0       J1 pin 7           PCS0      J2 pin 6
GND        J2 pin 14          GND       J2 pin 14
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1.  Connect a type-c USB cable between the host PC and the MCU-Link USB port (J13) on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Reset the SoC and run the project.

Running the demo
================
When the demo runs successfully, the log would be seen on the terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Master Start...
Succeed!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
