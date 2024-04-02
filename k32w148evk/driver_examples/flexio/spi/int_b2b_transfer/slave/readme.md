Overview
========
The flexio_spi_slave_interrupt example shows how to use flexio spi slave  driver in interrupt way:

In this example, a flexio simulated slave connect to a flexio simulated spi master.

SDK version
===========
- Version: 2.15.000

Toolchain supported
===================
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- K32W148-EVK Board
- Personal Computer

Board settings
==============
To make the example work, connections needed to be as follows:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MASTER(FlexIO SPI)          connect to      SLAVE(FlexIO SPI)
Pin Name   Board Location                   Pin Name    Board Location
SOUT       J1 pin 7                         SIN         J1 pin 7
SIN        J1 pin 4                         SOUT        J1 pin 4
SCK        J1 pin 8                         SCK         J1 pin 8
PCS0       J1 pin 3                         PCS0        J1 pin 3
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the EVK board J14.
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
FlexIO SPI interrupt example
Slave is working...

Slave runs successfully!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
