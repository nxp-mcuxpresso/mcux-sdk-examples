Overview
========
The spi_interrupt_master example shows how to use spi functional API to do interrupt transfer as a master:

In this example, the spi instance as master. Master sends a piece of data to slave, receive data from slave,
and check if the data master received is correct. This example needs to work with spi_interrupt_slave example.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Micro USB cable
- Two LPC845 Breakout boards
- Personal Computer

Board settings
==============
Pin connecting:  
Connect SPI master on board to SPI slave on other board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Master - SPI0                        Slave - SPI0
Pin Name   Board Location            Pin Name   Board Location
MISO       CN1 pin 12 (PIO0_26)      MISO       CN1 pin 12 (PIO0_26)
MOSI       CN1 pin 13 (PIO0_27)      MOSI       CN1 pin 13 (PIO0_27)
SCK        CN1 pin 6  (PIO0_21)      SCK        CN1 pin 6  (PIO0_21)
SSEL0      CN1 pin 5  (PIO0_20)      SSEL0      CN1 pin 5  (PIO0_20)
GND        CN1 pin 20                GND        CN1 pin 20
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the demo
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port on the board.
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
The log below shows the output of the SPI interrupt master driver example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
This is SPI interrupt functional master example.

Master start to send data to slave, please make sure the slave has been started!

The received data are:
  0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0A  0x0B  0x0C  0x0D  0x0E  0x0F
  0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17  0x18  0x19  0x1A  0x1B  0x1C  0x1D  0x1E  0x1F
  0x20  0x21  0x22  0x23  0x24  0x25  0x26  0x27  0x28  0x29  0x2A  0x2B  0x2C  0x2D  0x2E  0x2F
  0x30  0x31  0x32  0x33  0x34  0x35  0x36  0x37  0x38  0x39  0x3A  0x3B  0x3C  0x3D  0x3E  0x3F
Master transfer succeed!

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
