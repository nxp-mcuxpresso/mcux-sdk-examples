Overview
========
The freertos_lpspi_b2b_master example shows how to use LPSPI driver in FreeRTOS.
In this example are required two boards, one board is used as LPSPI master on which
is runnuing freertos_lpspi_b2b_master and another board is used as LPSPI slave on which
is running freertos_lpspi_b2b_slave example.

Connection of boards is in section Board settings.

It is required to run first the slave demo.



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


Running the demo
================
When the example runs successfully, you can see the similar information from the terminal as below.
If runs on one single board:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FreeRTOS LPSPI master example starts.
This example uses two boards to connect with one as master and anohter as slave.
Master and slave are both use interrupt way.
Please make sure you make the correct line connection. Basically, the connection is:
LPSPI_master -- LPSPI_slave
    CLK      --    CLK
    PCS      --    PCS
    SOUT     --    SIN
    SIN      --    SOUT

LPSPI master transfer completed successfully.
LPSPI transfer all data matched !
~~~~~~~~~~~~~~~
