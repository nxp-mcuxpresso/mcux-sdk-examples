Overview
========
The spi_polling_board2board_slave example shows how to use spi driver as slave to do board to board transfer with
polling:

Notice: The SPI slave of this example uses interrupt mode, as there is no polling mode for SPI slave.

In this example, one spi instance as slave and another spi instance on other board as master. Master sends a piece of
data to slave, and receive a piece of data from slave. This example checks if the data received from master is correct.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Micro USB cable
- Two LPCXpresso55S06 boards
- Personal Computer

Board settings
==============
Connect SPI slave on board to SPI master on other board
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
Slave is working....

Succeed!
​~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
