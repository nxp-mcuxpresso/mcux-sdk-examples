Overview
========
The spi_interrupt_board2board_master example shows how to use spi driver as master to do board to board transfer with
interrupt:

In this example, one spi instance as master and another spi instance on othereboard as slave. Master sends a piece of
data to slave, and receive a piece of data from slave. This example checks if the data received from slave is correct.

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
- Two LPCXpresso55S06 boards
- Personal Computer

Board settings
==============
SPI one board:
Transfer data from SPI master interface on board to 
other SPI interface on other board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Master_board(SPI3)           Slave_board(SPI3)                          
Pin Name   Board Location     Pin Name   Board Location                     
MISO       J12   pin 2        MISO       J12   pin 2
MOSI       J12   pin 1        MOSI       J12   pin 1
SCK        J10   pin 16       SCK        J10   pin 16
SSEL0      J10   pin 14       SSEL0      J10   pin 14
GND        J9    pin 8        GND        J9    pin 8
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    
Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the LPC-Link USB port (J1) on the board.
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

​~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Master Start...!

Succeed!
​~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
