Overview
========
The spi_interrupt_b2b_master example shows how to use spi functional API to do interrupt transfer as a master:

In this example, the spi instance as master. Master sends a piece of data to slave, receive data from slave,and check
if the data master received is correct. This example needs to work with spi_interrupt_b2b_slave example.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
-Type-C USB cable
- Two FRDM-MCXC041 board
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
1.  Connect a Type-C USB cable between the PC host and the OpenSDA USB port on the board.
2.  Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 9600 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Reset the SoC and run the project.

Running the demo
================
When the demo runs successfully, the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SPI board to board interrupt slave example started!
SPI transfer finished!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
