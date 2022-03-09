Overview
========
The spi_interrupt example shows how to use spi functional API to do interrupt transfer:

In this example, one spi instance as master and another spi instance as slave. Master sends a piece of data to slave,
and check if the data slave received is correct.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso55S06 board
- Personal Computer

Board settings
==============
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Master_board(SPI7)           Slave_board(SPI3)                          
Pin Name   Board Location     Pin Name   Board Location                     
MISO       J9    pin 11       MISO       J12   pin 2
MOSI       J9    pin 9        MOSI       J12   pin 1
SCK        J9    pin 13       SCK        J10   pin 16
SSEL0      J12   pin 13       SSEL0      J10   pin 14
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    
Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J1) on the board
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Reset the SoC and run the project.

Running the demo
================
When the demo runs successfully, the log would be seen on the CMSIS DAP terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SPI one board interrupt example started!

SPI transfer finished!
​~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
