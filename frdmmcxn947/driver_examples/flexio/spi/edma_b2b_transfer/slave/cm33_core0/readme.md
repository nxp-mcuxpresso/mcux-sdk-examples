Overview
========
The flexio_spi_slave_edma example shows how to use flexio spi slave  driver in dma way:

In this example, a flexio simulated slave connect to a flexio simulated spi master.

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
- FRDM-MCXN947 Board
- Personal Computer

Board settings
==============
To make the example work, connections needed to be as follows:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MASTER(FlexIO SPI)          connect to      SLAVE(FlexIO SPI)
Pin Name   Board Location                   Pin Name    Board Location
SOUT       J8 pin 26                       SIN         J8 pin 26
SIN        J8 pin 25                       SOUT        J8 pin 25
SCK        J8 pin 27                       SCK         J8 pin 27
PCS0       J8 pin 28                       PCS0        J8 pin 28
GND        J8 pin 2                        GND         J8 pin 2
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the EVK board J17.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
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
FlexIO SPI edma example
Slave is working...

Slave runs successfully!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
