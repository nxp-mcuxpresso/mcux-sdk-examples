Overview
========
The spi_b2b_example_slave example shows how to use LPSPI driver.

This example needs two boards, one is used as LPSPI master runnuing the spi_b2b_example_master
example, and the other is used as LPSPI slave running the spi_b2b_example_slave example.

Start the spi_b2b_example_slave example first, then start the spi_b2b_example_master example.


SDK version
===========
- Version: 2.15.100

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- Two MIMXRT1060-EVKC boards
- Personal Computer
- 5-pin flat cable

Board settings
==============
Weld 0Ω resistor to R346,R350,R356,R362.

SPI1 pins are connected with SPI1 pins of another board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
BOARD1(LPSPI1)          CONNECTS TO     BOARD2(LPSPI1)
Pin Name   Board Location     Pin Name  Board Location
PCS0        J17 pin 3           PCS0      J17 pin 3 
SOUT        J17 pin 4           SIN       J17 pin 5
SIN         J17 pin 5           SOUT      J17 pin 4
SCK         J17 pin 6           SCK       J17 pin 6
GND         J17 pin 7           GND       J17 pin 7
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Note that one board works as SPI master, and the other works as SPI slave.

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
After starting the example, the serial port will output:

Start the SPI board-to-board slave example

Ready to receive ...
Slave received:
0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07
0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17
0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f

Slave transmited:
0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07
0x08  0x09  0x0a  0x0b  0x0c  0x0d  0x0e  0x0f
0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17
0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f

Master-to-slave data verified ok.

