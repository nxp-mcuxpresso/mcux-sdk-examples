Overview
========
The flexio_spi_master_interrupt example shows how to use flexio spi master  driver in interrupt way:

In this example, a flexio simulated master connect to a flexio simulated spi slave .

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXA156 Board
- Personal Computer

Board settings
==============
To make the example work, connections needed to be as follows:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MASTER(FlexIO SPI)          connect to      SLAVE(FlexIO SPI)
Pin Name   Board Location                   Pin Name    Board Location
SOUT       J8 pin 13                       SIN         J8 pin 13
SIN        J8 pin 14                       SOUT        J8 pin 14
SCK        J8 pin 15                       SCK         J8 pin 15
PCS0       J8 pin 16                       PCS0        J8 pin 16
GND        J8 pin 8                        GND         J8 pin 8
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the EVK board J21.
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
Master Start...

Master runs successfully!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
