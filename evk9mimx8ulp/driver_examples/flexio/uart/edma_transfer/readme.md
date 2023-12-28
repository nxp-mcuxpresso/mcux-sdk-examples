Overview
========
The flexio_spi_slave_interrupt_lpspi_master example shows how to use flexio spi slave driver in interrupt way:

In this example, a flexio simulated slave connect to a lpspi master.



SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- MIMX8ULP-EVK/EVK9 board
- J-Link Debug Probe
- 5V power supply
- Personal Computer

Board settings
==============
1.The example requires connecting the FLEXIO0 pins with the LPSPI1 pins;
Please use four dupont lines to connect pins on MIMX8ULP-EVK base board as following:
- ARDU_D3/PTB0_FXIO0_D16/PCS(J21,4), ARDU_A0/PTA15_LPSPI1_PCS0(J23,1) connected
- ARDU_D2/PTB1_FXIO0_D17/SOUT(J21,3), ARDU_A1/PTA16_LPSPI1_SIN(J23,2) connected
- ARDU_D1/PTB2_FXIO0_D18/SIN(J21,2), ARDU_A2/PTA17_LPSPI1_SOUT(J23,3) connected
- ARDU_D0/PTB3_FXIO0_D19/SCK(J21,1), ARDU_A3/PTA18_LPSPI1_SCK(J23,4) connected

2.Remove the resistors R139,R159 and add the resistors R140,R158 on base board;

#### Please note this application can't support running with Linux BSP! ####

#### Please note this application can only run well with RAM link file!
If run it in QSPI flash, there's high latency when instruction cache miss. As the FlexIO SPI has
no FIFO so it has critical timing requirement that SPI data must be read in time, otherwise
overflow may occur which causes data loss. To run FlexIO SPI slave in QSPI flash, either use DMA
driver or do synchronization for data exchange. ####

Prepare the Demo
================
1.  Connect 5V power supply and J-Link Debug Probe to the board, switch SW10 to power on the board.
2.  Connect a micro USB cable between the host PC and the J17 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
When the example runs successfully, you can see the similar information from the terminal as below.

~~~~~~~~~~~~~~~~~~~~~
LPSPI Master interrupt - FLEXIO SPI Slave interrupt example start.

This example use one lpspi instance as master and one flexio spi slave on one board.

Master and slave are both use interrupt way.

Please make sure you make the correct line connection. Basically, the connection is:

LPSPI_master -- FLEXIO_SPI_slave

   SCK      --    SCK

   PCS      --    PCS

   SOUT     --    SIN

   SIN      --    SOUT

This is FLEXIO SPI slave call back.

LPSPI master <-> FLEXIO SPI slave transfer all data matched!

End of Example.
~~~~~~~~~~~~~~~~~~~~~
