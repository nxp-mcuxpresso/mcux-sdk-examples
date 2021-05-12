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
- MCUXpresso  11.3.0

Hardware requirements
=====================
- Mini USB cable
- Two FRDM-K32L2A4S boards
- Personal Computer

Board settings
==============
SPI one board:
Transfer data from one board instance to another board's instance.
SPI0 pins are connected with SPI0 pins of another board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
INSTANCE0(SPI0)     CONNECTS TO         INSTANCE0(SPI0)
Pin Name   Board Location     Pin Name  Board Location
SOUT        J2 pin 8            SIN       J2 pin 10
SIN         J2 pin 10           SOUT      J2 pin 8
SCK         J2 pin 12           SCK       J2 pin 12
PCS0        J2 pin 6            PCS0      J2 pin 6
GND         J2 pin 14           GND       J2 pin 14
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1.  Connect a mini USB cable between the PC host and the OpenSDA USB port on the board.
2.  Open a serial terminal on PC for OpenSDA serial device with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Reset the SoC and run the project.


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
