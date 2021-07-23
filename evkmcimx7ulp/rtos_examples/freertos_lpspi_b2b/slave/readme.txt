Overview
========
The freertos_lpspi_b2b_slave example shows how to use LPSPI driver in FreeRTOS.
In this example are required two boards, one board is used as LPSPI master on which
is runnuing freertos_lpspi_b2b_master and another board is used as LPSPI slave on which
is running freertos_lpspi_b2b_slave example.

Connection of boards is in section Board settings.

It is required to run first the slave demo.



Toolchain supported
===================
- GCC ARM Embedded  10.2.1

Hardware requirements
=====================
- Micro USB cable
- two MCIMX7ULP-EVK boards
- J-Link Debug Probe
- 5V power supply
- Personal Computer

Board settings
==============
Populate R188, R189, R190, R191 on both base boards.

SPI one board:
Transfer data from one board instance to another board's instance.
SPI1 pins are connected with SPI1 pins of another board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
INSTANCE0(SPI1)     CONNECTS TO         INSTANCE0(SPI1)
Pin Name   Board Location     Pin Name  Board Location
SPI1_SCK    J8 pin 6            SPI1_SCK  J8 pin 6
SPI1_SIN    J8 pin 5            SPI1_SOUT J8 pin 4
SPI1_SOUT   J8 pin 4            SPI1_SIN  J8 pin 5
SPI1_PCS0   J8 pin 3            SPI1_PCS0 J8 pin 3
GND                             GND
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#### Please note this application can't support running with Linux BSP! ####
Note
~~~~~~~~~~~~~~
Because of the LPSPI signal pin allocating issue, you can not restart this demo/example by press the ResetButton(SW3):
The LPSPI1_SIN, LPSPI1_SOUT, LPSPI1_SCK and LPSPI1_PCS0 pins are connected to PTA12, PTA13, PTA14 and PTA15 pads.
These pins are also used as BOO_CFG12, BOO_CFG13, BOO_CFG14 and BOO_CFG15 when the i.MX SoC reboot. These signals are
driven by LPSPI1 pins on the other board, when user press ResetButton. This may cause M4 Core enter enter
incorrect boot modes and only re-powerup both boards can let the M4 core exit such modes.

Prepare the Demo
================
1.  Connect 5V power supply and J-Link Debug Probe to the board, switch SW1 to power on the board.
2.  Connect a micro USB cable between the host PC and the J6 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either cold boot your board or launch the debugger in your IDE to begin running the example.


Running the demo
================
When the example runs successfully, you can see the similar information from the terminal as below.
If runs on one single board:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FreeRTOS LPSPI slave example starts.
This example uses two boards to connect with one as master and anohter as slave.
Master and slave are both use interrupt way.
Please make sure you make the correct line connection. Basically, the connection is:
LPSPI_master -- LPSPI_slave
    CLK      --    CLK
    PCS      --    PCS
    SOUT     --    SIN
    SIN      --    SOUT

LPSPI slave transfer completed successfully.
LPSPI transfer all data matched !
~~~~~~~~~~~~~~~
