Overview
========
The cmsis_lpspi_edma_b2b_transfer example shows how to use LPSPI CMSIS driver in edma way:

In this example , we need two boards, one board used as LPSPI master and another board used as LPSPI slave.
The file 'cmsis_lpspi_edma_b2b_transfer_slave.c' includes the LPSPI slave code.

1. LPSPI master send/received data to/from LPSPI slave in edma . 

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Mini/micro USB cable
- EVKB-IMXRT1050 board
- Personal Computer

Board settings
==============
Remove the resistor R334 and weld 0Ω resistor to R278,R279,R280,R281.

LPSPI:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
INSTANCE1(LPSPI1)     CONNECTS TO         INSTANCE1(LPSPI1)
Pin Name   Board Location     Pin Name  Board Location
SOUT        J24 pin 4           SIN       J24 pin 5
SIN         J24 pin 5           SOUT      J24 pin 4
SCK         J24 pin 6           SCK       J24 pin 6
PCS0        J24 pin 3           PCS0      J24 pin 3 
GND         J24 pin 7           GND       J24 pin 7
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
4. Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs successfully, you can see the similar information from the terminal as below.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LPSPI CMSIS interrupt transfer example start.

This example use one lpspi instance as master and another as slave on one board.

Master uses interrupt way and slave uses interrupt way.

Note that some LPSPI instances interrupt is in INTMUX ,you should set the intmux when you porting this example accordingly

Please make sure you make the correct line connection. Basically, the connection is:

LPSPI_master -- LPSPI_slave

   CLK      --    CLK

   PCS      --    PCS

   SOUT     --    SIN

   SIN      --    SOUT

This is SlaveSignalEvent.

LPSPI transfer all data matched!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
