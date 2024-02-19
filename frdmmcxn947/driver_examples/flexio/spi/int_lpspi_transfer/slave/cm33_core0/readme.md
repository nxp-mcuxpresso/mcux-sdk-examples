Overview
========
The flexio_spi_slave_interrupt_lpspi_master example shows how to use flexio spi slave driver in interrupt way:

In this example, a flexio simulated slave connect to a lpspi master.



SDK version
===========
- Version: 2.14.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXN947 Board
- Personal Computer

Board settings
==============
To make the example work, connections needed to be as follows:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MASTER(LPSPI1)              connect to      SLAVE(FlexIO SPI)
Pin Name   Board Location                   Pin Name    Board Location
SOUT       J2 pin 10                        SIN         J8 pin 26
SIN        J2 pin 8                         SOUT        J8 pin 25
SCK        J2 pin 12                        SCK         J8 pin 27
PCS0       J2 pin 6                         PCS0        J8 pin 28
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
LPSPI Master interrupt - FLEXIO SPI Slave interrupt example start.
This example use one lpspi instance as master and one flexio spi slave on one board.
Master and slave are both use interrupt way.
Please make sure you make the correct line connection. Basically, the connection is: 
LPSPI_master -- FLEXIO_SPI_slave   
   CLK      --    CLK  
   PCS      --    PCS  
   SOUT     --    SIN  
   SIN      --    SOUT 
This is FLEXIO SPI slave call back.
LPSPI master <-> FLEXIO SPI slave transfer all data matched!

End of Example. 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
