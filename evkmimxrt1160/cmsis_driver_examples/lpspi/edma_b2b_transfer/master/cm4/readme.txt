Overview
========
The cmsis_lpspi_edma_b2b_transfer example shows how to use LPSPI CMSIS driver in edma way:

In this example , we need two boards, one board used as LPSPI master and another board used as LPSPI slave.
The file 'cmsis_lpspi_edma_b2b_transfer_master.c' includes the LPSPI master code.

1. LPSPI master send/received data to/from LPSPI slave in edma . 

Toolchain supported
===================
- MCUXpresso  11.4.0
- GCC ARM Embedded  10.2.1

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1160-EVK board
- Personal Computer

Board settings
==============
Remove 0Ω resistors R404,R406,R408.

LPSPI:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
       MASTER           connect to           SLAVE
Pin Name   Board Location     Pin Name    Board Location
SOUT       J10-8                SIN       J10-10
SIN        J10-10               SOUT      J10-8
SCK        J10-12               SCK       J10-12
PCS0       J10-6                PCS0      J10-6
GND        J10-14               GND       J10-14
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
LPSPI CMSIS driver board to board edma example.
This example use one board as master and another as slave.
Master and slave uses EDMA way. Slave should start first. 
Please make sure you make the correct line connection. Basically, the connection is: 
LPSPI_master -- LPSPI_slave   
   CLK       --    CLK  
   PCS       --    PCS 
   SOUT      --    SIN  
   SIN       --    SOUT 
   GND       --    GND 

 Master transmit:

  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 10
 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20
 21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30
 31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F 40
This is LPSPI_MasterSignalEvent_t
Master transmit data to slave has completed!
This is LPSPI_MasterSignalEvent_t
Master receive data from slave has completed!
 
LPSPI transfer all data matched! 

 Master received:

  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 10
 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20
 21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30
 31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F 40

 Input any char to run again
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
