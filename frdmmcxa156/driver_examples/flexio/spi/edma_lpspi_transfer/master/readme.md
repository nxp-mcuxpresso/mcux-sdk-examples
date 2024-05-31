Overview
========
The flexio_spi_master_edma_lpspi_slave example shows how to use flexio spi master driver in edma way:

In this example, a flexio simulated master connect to a lpspi slave .



SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.3
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXA156 Board
- Personal Computer

Board settings
==============
To make the example work, connections needed to be as follows:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MASTER(FlexIO SPI)          connect to      SLAVE(LPSPI0)
Pin Name   Board Location                   Pin Name    Board Location
SOUT       J8 pin 14                       SIN         J6 pin 5
SIN        J8 pin 13                       SOUT        J6 pin 6
SCK        J8 pin 15                       SCK         J6 pin 4
PCS0       J8 pin 16                       PCS0        J6 pin 3
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
FLEXIO Master edma - LPSPI Slave interrupt example start.
This example use one flexio spi as master and one lpspi instance as slave on one board.
Master uses edma and slave uses interrupt way.
Please make sure you make the correct line connection. Basically, the connection is:
FLEXIO_SPI_master -- LPSPI_slave   
       CLK        --    CLK  
       PCS        --    PCS  
       SOUT       --    SIN  
       SIN        --    SOUT 
This is LPSPI slave call back.
FLEXIO SPI master <-> LPSPI slave transfer all data matched!

End of example.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
