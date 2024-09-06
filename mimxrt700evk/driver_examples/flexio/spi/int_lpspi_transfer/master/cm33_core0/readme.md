Overview
========
The flexio_spi_master_interrupt_lpspi_slave example shows how to use flexio spi master driver in interrupt way:

In this example, a flexio simulated master connect to a lpspi slave .



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
- MIMXRT700-EVK board
- Personal Computer

Board settings
==============
To make the example work, connections needed to be as follows:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 SLAVE(LPSPI16) connect to               MASTER(FlexIO0)
Pin Name   Board Location            Pin Name  Board Location
SOUT        J22 pin 6         	     SIN       J5 pin 3
SIN         J22 pin 5         	     SOUT      J5 Pin 6
SCK         J22 pin 4        	     SCK       J5 pin 4
PCS0        J22 pin 3        	     PCS       J5 pin 5
GND         J22 pin 8                GND       J6 pin 7
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1. Connect a mini USB cable between the PC host and the MCU-LINK USB port on the board.
2. Open a serial terminal on PC for MCU-LINK serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3. Download the program to the target board.
4. Either press the reset button on your board or launch the debugger in your IDE to begin running
   the demo.

Running the demo
================
When the demo runs successfully, the log would be seen on the terminal:

~~~~~~~~~~~~~~~~~~~~~
FLEXIO Master - LPSPI Slave interrupt example start.
This example use one flexio spi as master and one lpspi instance as slave on one board.
Master and slave are both use interrupt way.
Please make sure you make the correct line connection. Basically, the connection is:
FLEXIO_SPI_master -- LPSPI_slave   
       CLK        --    CLK  
       PCS        --    PCS  
       SOUT       --    SIN  
       SIN        --    SOUT 
This is LPSPI slave call back.
FLEXIO SPI master <-> LPSPI slave transfer all data matched!

End of example.
~~~~~~~~~~~~~~~~~~~~~
