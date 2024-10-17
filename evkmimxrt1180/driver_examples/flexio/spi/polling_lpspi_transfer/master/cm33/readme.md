Overview
========
The flexio_spi_master_pooling_lpspi_slave example shows how to use flexio spi master driver in polling way:

In this example, a flexio simulated master connect to a lpspi slave.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1180-EVK board
- Personal Computer

Board settings
==============
To make the example work, connections needed to be as follows:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        SLAVE           connect to      MASTER
Pin Name   Board Location     Pin Name    Board Location
SCK        J44-12               SCK       J41-2
SIN        J44-10               SOUT      J41-6
SOUT       J44-8                SIN       J41-4
PCS0       J44-6                PCS0      J41-8
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1. Connect a mini USB cable between the PC host and the OpenSDA USB port on the board.
2. Open a serial terminal on PC for OpenSDA serial device with these settings:
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
You can see the similar message shows following in the terminal if the example runs successfully.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FLEXIO Master - LPSPI Slave polling example start.
This example use one flexio spi as master and one lpspi instance as slave on one board.
Master uses polling and slave uses interrupt way.
Please make sure you make the correct line connection. Basically, the connection is:
FLEXIO_SPI_master -- LPSPI_slave
      CLK        --    CLK
      PCS        --    PCS
      SOUT       --    SIN
      SIN        --    SOUT

This is LPSPI slave call back.
FLEXIO SPI master <-> LPSPI slave transfer all data matched!
End of example.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
