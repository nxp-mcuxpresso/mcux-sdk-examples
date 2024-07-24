Overview
========
The spi_interrupt_board2board_slave example shows how to use spi driver as slave to do board to board transfer with 
interrupt:

In this example, one spi instance as slave and another spi instance on other board as master. Master sends a piece of
data to slave, and receive a piece of data from slave. This example checks if the data received from master is correct.

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
- RD-RW61X-BGA board
- Personal Computer

Board settings
==============
Connect JP30 1-2, JP9, JP19, JP23, JP51. Disconnect JP47 and connect JP47-1 to GND(J8-6).
Extra rework is needed for the slave board besides the above connection, remove R97, R415, R594
and install R409, R49, R13.

Connect SPI master on board to SPI slave on other board.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Master_board(SPI0)           Slave_board(SPI0)
Pin Name   Board Location     Pin Name   Board Location
MISO       J5 pin 5           MISO       J5 pin 5
MOSI       J5 pin 4           MOSI       J5 pin 4
SCK        J5 pin 6           SCK        J5 pin 6
PCS0       J5 pin 3           PCS0       J5 pin 3
GND        J5 pin 7           GND        J5 pin 7
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J7) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs successfully, the log would be seen on the terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Slave is working....

Succeed!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
