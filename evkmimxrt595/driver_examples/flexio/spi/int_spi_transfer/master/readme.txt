Overview
========
The flexio_spi_master_interrupt_spi_slave example shows how to use flexio spi master driver in interrupt way:

In this example, a flexio simulated master connect to a spi slave .

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- EVK-MIMXRT595 board
- Personal Computer

Board settings
==============
To make the example work, connections needed to be as follows:
Note: Please disconnect JS23 1-2, and connect JS23-2 to JP23-3 to provide 1.8V to VDDIO_3
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 SLAVE(SPI5) connect to      MASTER(FlexIO0)
Pin Name   Board Location     Pin Name    Board Location
MOSI       JP26-2               MOSI        J28-4
MISO       JP26-3               MISO        J28-5
SCK        JP26-4               SCK         J28-6
PCS0       JP26-1               PCS0        J28-3
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
When the demo runs successfully, the log would be seen on the terminal:

~~~~~~~~~~~~~~~~~~~~~
FLEXIO Master - SPI Slave interrupt example start.
This example use one flexio spi as master and one spi instance as slave on one board.
Master and slave are both use interrupt way.
Please make sure you make the correct line connection. Basically, the connection is:
FLEXI_SPI_master -- SPI_slave   
   SCK      --    SCK  
   PCS0     --    PCS0 
   MOSI     --    MOSI 
   MISO     --    MISO 
This is SPI slave call back.
FLEXIO SPI master <-> SPI slave transfer all data matched!
~~~~~~~~~~~~~~~~~~~~~
