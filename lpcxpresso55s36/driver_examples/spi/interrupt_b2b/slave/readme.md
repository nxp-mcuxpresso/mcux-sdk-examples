Overview
========
The spi_interrupt_b2b_slave example shows how to use spi functional API to do interrupt transfer as a slave:

In this example, the spi instance as slave. Slave receives data froma master and send a peiece of data to master,
and check if the data slave received is correct. This example needs to work with spi_interrupt_b2b_master example.

SDK version
===========
- Version: 2.15.100

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- Two LPCXpresso55S36 boards
- Personal Computer

Board settings
==============
Short JP51 1-2, JP52 1-2.

SPI one board:
Transfer data from SPI slave interface on board to 
other SPI interface on other board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Master_board(SPI7)           Slave_board(SPI7)                          
Pin Name   Board Location     Pin Name   Board Location                     
MISO       J122  pin 12       MISO       J122  pin 12 
MOSI       J8    pin 3        MOSI       J8    pin 3
SCK        J122  pin 8        SCK        J122  pin 8
SSEL0      J122  pin 6        SSEL0      J122  pin 6
GND        J92   pin 14       GND        J92   pin 14                   
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    
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
SPI board to board interrupt slave example started!

SPI transfer finished!
​~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
