Overview
========
The lpspi_interrupt example shows how to use LPSPI driver in interrupt way:

In this example , one lpspi instance used as LPSPI master and another lpspi instance used as LPSPI slave.

1. LPSPI master send/received data to/from LPSPI slave in interrupt . (LPSPI Slave using interrupt to receive/send the data)

The example supports board to board connection.

With board to board connection, one LPSPI instance on one board is used as LPSPI master and the LPSPI instance on other board is used as LPSPI slave. Tasks are created to run on each board to handle LPSPI communication.
    File freertos_lpspi.c should have following definitions:
    #define EXAMPLE_CONNECT_SPI BOARD_TO_BOARD
    For board used as LPSPI master:
        #define SPI_MASTER_SLAVE isMASTER
    For board used as LPSPI slave:
        #define SPI_MASTER_SLAVE isSLAVE


Toolchain supported
===================
- GCC ARM Embedded  9.3.1
- MCUXpresso  11.3.0

Hardware requirements
=====================
- Micro USB cable
- FRDM-KE16Z board
- Personal Computer

Board settings
==============
SPI two boards:
Transfer data from one board's instance to another board's instance.
SPI0 pins are connected with SPI0 pins of another board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
INSTANCE(SPI0)     CONNECTS TO         INSTANCE(SPI0)
Pin Name   Board Location    Pin Name    Board Location
SOUT        J4 pin 7            SIN       J4 pin 5
SIN         J4 pin 5            SOUT      J4 pin 7
SCK         J4 pin 3            SCK       J4 pin 3
PCS3        J4 pin 6            PCS3      J4 pin 6
GND         J2 pin 14           GND       J2 pin 14
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
In file freertos_lpspi.c, do following definition:
            #define EXAMPLE_CONNECT_SPI BOARD_TO_BOARD
            For master, use following definition
                #define SPI_MASTER_SLAVE isMASTER
                Build project.
                Download the program to one target board (used as master board).
            For slave, use following definition
                #define SPI_MASTER_SLAVE isSLAVE
                Build project.
                Download the program to one target board (used as slave board). 
            Note: Slave side should run before Master side.

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
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
When the example runs successfully, you can see the similar information from the terminal as below.
If runs on one single board:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FreeRTOS LPSPI example start.
This example use one lpspi instance as master and another as slave on a single board.
Master and slave are both use interrupt way.
Please make sure you make the correct line connection. Basically, the connection is:
LPSPI_master -- LPSPI_slave
    CLK      --    CLK
    PCS      --    PCS
    SOUT     --    SIN
    SIN      --    SOUT

LPSPI master transfer completed successfully.
LPSPI slave transfer completed successfully.
LPSPI transfer all data matched !
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
If runs on two boards, slave should start first.
Master side:
~~~~~~~~~~~~
FreeRTOS LPSPI example start.
This example use two boards to connect with one as master and anohter as slave.
Master and slave are both use interrupt way.
Please make sure you make the correct line connection. Basically, the connection is:
LPSPI_master -- LPSPI_slave
    CLK      --    CLK
    PCS      --    PCS
    SOUT     --    SIN
    SIN      --    SOUT

LPSPI master transfer completed successfully.
LPSPI transfer all data matched !
~~~~~~~~~~~~~
SLAVE side:
~~~~~~~~~~~~~
FreeRTOS LPSPI example start.
This example use two boards to connect with one as master and anohter as slave.
Master and slave are both use interrupt way.
Please make sure you make the correct line connection. Basically, the connection is:
LPSPI_master -- LPSPI_slave
    CLK      --    CLK
    PCS      --    PCS
    SOUT     --    SIN
    SIN      --    SOUT

LPSPI slave transfer completed successfully.
LPSPI transfer all data matched !
~~~~~~~~~~~~~~~
