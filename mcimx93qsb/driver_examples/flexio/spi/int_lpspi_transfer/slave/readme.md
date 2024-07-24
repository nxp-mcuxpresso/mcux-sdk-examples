Overview
========
The flexio_spi_slave_interrupt_lpspi_master example shows how to use flexio spi slave driver in interrupt way:

In this example, a flexio simulated slave connect to a lpspi master.



SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- USB Type-C cable
- MCIMX93-QSB board
- J-Link Debug Probe
- 12V~20V power supply
- Personal Computer

Board settings
==============
The example requires doing connection between LPSPI3(as master) pins and FLEXIO1(as slave) pins.
- IOMUXC_PAD_GPIO_IO09__LPSPI3_SIN(J1401-21),  FLEXIO_SPI_SOUT_PIN(J1401-3) connected
- IOMUXC_PAD_GPIO_IO10__LPSPI3_SOUT(J1401-19), FLEXIO_SPI_SIN_PIN(J1401-5) connected
- IOMUXC_PAD_GPIO_IO11__LPSPI3_SCK(J1401-23),  FLEXIO_SPI_CLK_PIN(J1401-7) connected
- IOMUXC_PAD_GPIO_IO08__LPSPI3_PCS0(J1401-24), FLEXIO_SPI_PCS_PIN(J1401-29) connected

Note
Please run the application in Low Power boot mode (without Linux BSP).
The IP module resource of the application is also used by Linux BSP.
Or, run with Single Boot mode by changing Linux BSP to avoid resource
conflict.

Prepare the Demo
================
1.  Connect 12V~20V power supply and J-Link Debug Probe to the board, switch SW301 to power on the board.
2.  Connect a USB Type-C cable between the host PC and the J1708 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either re-power up your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
When the example runs successfully, you can see the similar information from the terminal as below.

~~~~~~~~~~~~~~~~~~~~~
LPSPI Master interrupt - FLEXIO SPI Slave interrupt example start.
This example use one lpspi instance as master and one flexio spi slave on one board.
Master and slave are both use interrupt way.
Please make sure you make the correct line connection. Basically, the connection is:
LPSPI_master -- FLEXIO_SPI_slave
   CLK      --    CLK
   PCS      --    PCS
   SOUT     --    SIN
   SIN      --    SOUT
This is FLEXIO SPI slave call back.
LPSPI master <-> FLEXIO SPI slave transfer all data matched!

End of Example.
~~~~~~~~~~~~~~~~~~~~~
