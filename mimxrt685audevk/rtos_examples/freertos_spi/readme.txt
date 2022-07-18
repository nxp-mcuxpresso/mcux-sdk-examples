Overview
========
The freertos_spi example shows how to use SPI driver in FreeRTOS:

In this example , one spi instance is used as SPI master with blocking and another spi instance is used as SPI slave.

1. SPI master sends/receives data using task blocking calls to/from SPI slave. (SPI Slave uses interrupt to receive/send
the data)


Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Micro USB cable
- MIMXRT685-AUD-EVK board
- Personal Computer

Board settings
==============
Connect SPI5 pins to SPI4 pins:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
INSTANCE0(SPI5)     CONNECTS TO         INSTANCE1(SPI4)
Pin Name   Board Location     Pin Name  Board Location
MISO       J28 pin 5          MISO      J27 pin 2
MOSI       J28 pin 4          MOSI      J27 pin 1
SCK        J28 pin 6          SCK       J27 pin 3
PCS0       J28 pin 3          PCS1      J27 pin 5
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Make sure the JP41 2-3 ,JP42 2-3 are connected. 
Prepare the Demo
================
Connect a serial cable from the debug UART port of the board to the PC. Start Tera Term
(http://ttssh2.osdn.jp) and make a connection to the virtual serial port.

1. Start Tera Term
2. New connection -> Serial
3. Set appropriate COMx port (x is port number) in Port context menu. Number is provided by operation
   system and could be different from computer to computer. Select COM number related to virtual
   serial port. Confirm selected port by OK button.
4. Set following connection parameters in menu Setup->Serial port...
        Baud rate:    115200
        Data:         8
        Parity:       none
        Stop:         1
        Flow control: none
5.  Confirm selected parameters by OK button.

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
