Overview
========
The spi_interrupt_b2b_slave example shows how to use spi functional API to do interrupt transfer as a slave:

In this example, the spi instance as slave. Slave receives data froma master and send a peiece of data to master,
and check if the data slave received is correct. This example needs to work with spi_interrupt_b2b_master example.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT595 board
- Personal Computer

Board settings
==============

The example provides 2 options of SPI interfaces in communication.
1. SPI5 (default, normal SPI: #define USE_HS_SPI 0)
2. SPI14 (optional, high speed SPI: #define USE_HS_SPI 1)
User can change the USE_HS_SPI value in spi_interrupt_b2b_slave.c to choose different SPI interface.

Connect SPI master on board to SPI slave on other board. Only 1 set of signals need to be connected, either
SPI5 or SPI14 depending on the USE_HS_SPI setting.
NOTE: To use SPI14 on EVK board, please connect JP32 1-2.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Master_board(SPI14)             Slave_board(SPI14) 
Pin Name   Board Location     Pin Name   Board Location
MISO       J36 pin 5          MISO       J36 pin 5
MOSI       J36 pin 3          MOSI       J36 pin 3
SCK        J36 pin 7          SCK        J36 pin 7
PCS0       J36 pin 1          PCS0       J36 pin 1
GND        J36 pin 9          GND        J36 pin 9
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Connect SPI master on board to SPI slave on other board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Master_board(SPI5)           Slave_board(SPI5)                          
Pin Name   Board Location     Pin Name   Board Location                     
MISO       JP26 pin 3          MISO      JP26 pin 3  
MOSI       JP26 pin 2          MOSI      JP26 pin 2  
SCK        JP26 pin 4          SCK       JP26 pin 4  
PCS0       JP26 pin 1          PCS0      JP26 pin 1  
GND        JP26 pin 5          GND       JP26 pin 5  
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J40) on the board
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
SPI board to board interrupt slave example started!

SPI transfer finished!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
