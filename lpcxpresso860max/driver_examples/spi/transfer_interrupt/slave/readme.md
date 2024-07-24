Overview
========
The spi_interrupt_transfer_slave example shows how to use spi driver as slave to receive data from master.

In this example, one spi instance as slave and another spi instance on other board as master. 
Master sends a piece of data to slave, and receive a piece of data from slave. This example 
checks if the data received from master is correct. This example should work with 
spi_interrupt_transfer_master example. And this example should start first.
  

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
- Two LPCXpresso860MAX boards
- Personal Computer

Board settings
==============
Pin connecting:  
Connect SPI master on board to SPI slave on other board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Master - SPI0                  Slave - SPI0   
Pin Name   Board Location      Pin Name   Board Location            
MISO       J1 pin 10           MISO       J1 pin 10                     
MOSI       J1 pin 8            MOSI       J1 pin 8                
SCK        J1 pin 12           SCK        J1 pin 12                 
SSEL0      J1 pin 6            SSEL0      J1 pin 6
GND        J1 pin 14           GND        J1 pin 14
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port on the board
2.  Open a serial terminal with the following settings:
    - 9600 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows the output of the hello world demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This is SPI interrupt transfer slave example.

Slave is working....

The received data are:
  0x 0  0x 1  0x 2  0x 3  0x 4  0x 5  0x 6  0x 7  0x 8  0x 9  0x A  0x B  0x C  0x D  0x E  0x F
  0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17  0x18  0x19  0x1A  0x1B  0x1C  0x1D  0x1E  0x1F
  0x20  0x21  0x22  0x23  0x24  0x25  0x26  0x27  0x28  0x29  0x2A  0x2B  0x2C  0x2D  0x2E  0x2F
  0x30  0x31  0x32  0x33  0x34  0x35  0x36  0x37  0x38  0x39  0x3A  0x3B  0x3C  0x3D  0x3E  0x3F
Slave interrupt transfer succeed!


~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
