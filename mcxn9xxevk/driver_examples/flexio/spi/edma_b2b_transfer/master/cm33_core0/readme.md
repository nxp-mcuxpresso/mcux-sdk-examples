Overview
========
The flexio_spi_master_edma example shows how to use flexio spi master  driver in edma way:

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
- Mini/micro USB cable
- MCX-N9XX-EVK Board
- Personal Computer

Board settings
==============
To make the example work, connections needed to be as follows:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MASTER(FlexIO SPI)          connect to      SLAVE(FlexIO SPI)
Pin Name   Board Location                   Pin Name    Board Location
SOUT       J20 pin 26                       SIN         J20 pin 26
SIN        J20 pin 25                       SOUT        J20 pin 25
SCK        J20 pin 27                       SCK         J20 pin 27
PCS0       J20 pin 28                       PCS0        J20 pin 28
GND        J20 pin 2                        GND         J20 pin 2
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the EVK board J5.
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
Master Start...

Master runs successfully!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
