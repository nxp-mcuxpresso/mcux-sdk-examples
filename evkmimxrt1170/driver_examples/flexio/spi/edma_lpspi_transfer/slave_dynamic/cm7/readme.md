Overview
========
The flexio_spi_edma_lpspi_transfer_slave_dynamic example shows how to use flexio spi slave driver in edma way:

In this example, a flexio simulated slave connect to a lpspi master. The CS signal remains low during transfer,
after master finishes the transfer and re-asserts the CS signal, interrupt is triggered to let user know.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1170-EVK board
- Personal Computer

Board settings
==============
Remove 0Ω resistor to R200,R406,R408,R404.

To make the example work, connections needed to be as follows:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
       MASTER           connect to      SLAVE
Pin Name   Board Location     Pin Name    Board Location
SOUT       J10-10               SIN       J26-6
SIN        J10-8                SOUT      J26-4
SCK        J10-12               SCK       J26-2
PCS0       J10-6                PCS0      J26-8
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
LPSPI Master interrupt - FLEXIO SPI Slave edma dynamic transfer example start.
This example use one lpspi instance as master and one flexio spi slave on one board.
Master uses interrupt and slave uses edma way.
Master transfers indefinite amount of data to slave, and the CS signal remains low during transfer.
After master finishes the transfer and re-asserts the CS signal, interrupt is triggered to let user know.
Slave must configure the transfer size larger than master's.
Please make sure you make the correct line connection. Basically, the connection is:
LPSPI_master -- FLEXIO_SPI_slave
   CLK      --    CLK
   PCS      --    PCS
   SOUT     --    SIN
   SIN      --    SOUT
Slave received 128 bytes of data
Slave received 128 bytes of data
LPSPI master <-> FLEXIO SPI slave transfer all data matched!

End of Example.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
