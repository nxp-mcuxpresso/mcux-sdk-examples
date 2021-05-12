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

Hardware requirements
=====================
- Micro USB cable
- i.MX8QM MEK Board
- MCIMX8-8X-BB
- J-Link Debug Probe
- 12V power supply
- Personal Computer

Board settings
==============
SPI two boards:
Transfer data from one board's instance to another board's instance.
SPI2 pins are connected with SPI2 pins of another board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
INSTANCE(SPI2)     CONNECTS TO         INSTANCE(SPI2)
Pin Name  Base Board Location  Pin Name  Base Board Location
SOUT        J29-6              SIN       J29-5
SIN         J29-5              SOUT      J29-6
SCK         J29-4              SCK       J29-4
PCS0        J29-3              PCS0      J29-3
GND         J29-8              GND       J29-8
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

#### Please note this application can't support running with Linux BSP! ####
This example aims to show the basic usage of the IP's function, some of the used Pads/Resources are also used by Cortex-A core.

Prepare the Demo
================
1.  Connect 12V power supply and J-Link Debug Probe to the board.
2.  Connect a USB cable between the host PC and the Debug port on the board (Refer "Getting Started with MCUXpresso SDK for i.MX 8QuadMax.pdf" for debug port information).
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board (Please refer "Getting Started with MCUXpresso SDK for i.MX 8QuadMax.pdf" for how to run different targets).
5.  Launch the debugger in your IDE to begin running the example.

