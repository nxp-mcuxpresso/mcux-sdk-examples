Overview
========
The lpspi_functional_interrupt example shows how to use LPSPI driver in interrupt way:

In this example , one lpspi instance used as LPSPI master and another lpspi instance used as LPSPI slave .

1. LPSPI master send/received data to/from LPSPI slave in interrupt . (LPSPI Slave using interrupt to receive/send the data)



SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXN947 board
- Personal Computer

Board settings
==============

LPSPI:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
       MASTER           connect to           SLAVE
Pin Name   Board Location     Pin Name    Board Location
SOUT       J2-18                SIN       J2-10
SIN        J1-4                 SOUT      J2-8
SCK        J2-20                SCK       J2-12
PCS0       J1-2                 PCS0      J2-6
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1. Connect a mini USB cable between the PC host and the MCU-Link USB port on the board.
2. Open a serial terminal on PC for MCU-Link serial device with these settings:
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
When the example runs successfully, you can see the similar information from the terminal as below.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LPSPI functional interrupt example start.

This example use one lpspi instance as master and another as slave on one board.

Master uses interrupt way and slave uses interrupt way.

Note that some LPSPI instances interrupt is in INTMUX ,you should set the intmux when you porting this example accordingly

Please make sure you make the correct line connection. Basically, the connection is:

LPSPI_master -- LPSPI_slave

   CLK      --    CLK

   PCS      --    PCS

   SOUT     --    SIN

   SIN      --    SOUT

LPSPI transfer all data matched!

End of example.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
