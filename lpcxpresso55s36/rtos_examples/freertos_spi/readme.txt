Overview
========
The freertos_spi example shows how to use SPI driver in FreeRTOS:

In this example , one spi instance is used as SPI master with blocking and another spi instance is used as SPI slave.

1. SPI master sends/receives data using task blocking calls to/from SPI slave. (SPI Slave uses interrupt to receive/send
the data)


Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.4.1

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso55S36 board
- Personal Computer

Board settings
==============
Connect SPI8 pins to SPI2 pins:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Master_board(SPI2)           Slave_board(SPI8)                          
Pin Name   Board Location     Pin Name   Board Location                     
MISO       J122  pin 12       MISO       J7    pin 5 
MOSI       J8    pin 3        MOSI       J7    pin 6
SCK        J122  pin 8        SCK        J7    pin 4
SSEL0      J122  pin 6        SSEL0      J7    pin 3
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Short JP51 1-2, JP52 1-2, remove JP48.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J1) on the board
2.  Open a serial terminal with the following settings (See Appendix A in Getting started guide for description how to determine serial port number):
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs successfully, the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FreeRTOS SPI example start.
This example use one SPI instance as master and another as slave on one board.
Master and slave are both use interrupt way.
Please make sure you make the correct line connection. Basically, the connection is:
SPI_master -- SPI_slave
   CLK      --    CLK
   PCS0     --    PCS0
   SOUT     --    SIN
   SIN      --    SOUT
Master transmited:
0x 0  0x 1  0x 2  0x 3  0x 4  0x 5  0x 6  0x 7
0x 8  0x 9  0x a  0x b  0x c  0x d  0x e  0x f
0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17
0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f

Slave received:
0x 0  0x 1  0x 2  0x 3  0x 4  0x 5  0x 6  0x 7
0x 8  0x 9  0x a  0x b  0x c  0x d  0x e  0x f
0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17
0x18  0x19  0x1a  0x1b  0x1c  0x1d  0x1e  0x1f

Slave transmited:
0xff  0xfe  0xfd  0xfc  0xfb  0xfa  0xf9  0xf8
0xf7  0xf6  0xf5  0xf4  0xf3  0xf2  0xf1  0xf0
0xef  0xee  0xed  0xec  0xeb  0xea  0xe9  0xe8
0xe7  0xe6  0xe5  0xe4  0xe3  0xe2  0xe1  0xe0

Master received:
0xff  0xfe  0xfd  0xfc  0xfb  0xfa  0xf9  0xf8
0xf7  0xf6  0xf5  0xf4  0xf3  0xf2  0xf1  0xf0
0xef  0xee  0xed  0xec  0xeb  0xea  0xe9  0xe8
0xe7  0xe6  0xe5  0xe4  0xe3  0xe2  0xe1  0xe0

SPI master transfer completed successfully.
Slave-to-master data verified ok.
SPI slave transfer completed successfully.
Master-to-slave data verified ok.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
