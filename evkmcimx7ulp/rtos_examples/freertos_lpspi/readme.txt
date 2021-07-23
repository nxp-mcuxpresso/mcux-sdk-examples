Overview
========
The lpspi_interrupt example shows how to use LPSPI driver in interrupt way:

In this example , one lpspi instance used as LPSPI master and another lpspi instance used as LPSPI slave.

1. LPSPI master send/received data to/from LPSPI slave in interrupt . (LPSPI Slave using interrupt to receive/send the data)

The example supports board to board connection.

With board to board connection, one LPSPI instance on one board is used as LPSPI master and the LPSPI instance on other board is used as LPSPI slave. Tasks are created to run on each board to handle LPSPI communication.
    File freertos_lpspi.c should have following definitions:
    #define EXAMPLE_CONNECT_SPI BOARD_TO_BOARD
    For board used as LPSPI master:
        #define SPI_MASTER_SLAVE isMASTER
    For board used as LPSPI slave:
        #define SPI_MASTER_SLAVE isSLAVE


Toolchain supported
===================
- GCC ARM Embedded  10.2.1

Hardware requirements
=====================
- Micro USB cable
- MCIMX7ULP-EVK board
- J-Link Debug Probe
- 5V power supply
- Personal Computer

Board settings
==============
The example requires doing following rework:
1.Remove the resistors R101, R19 on base board;

Note:
Remove R101,R19 to avoid some other signals may have interference on LPSPI,
if users just run this application to demonstrate LPSPI function and no other applicaions are running at the same time,
then R101 & R19 do not need to be removed.

please connect between LPSPI1 pins and LPSPI0 pins
The connection should be set as following:
- J8-3(R191 should be short-circuited), R101 pad1(On base board) connected
- J8-4(R190 should be short-circuited), R22(On base board) connected
- J8-5(R189 should be short-circuited), TP25(On base board) connected
- J8-6(R188 should be short-circuited), TP27(On base board) connected

#### Please note this application can't support running with Linux BSP! ####

#### Please note there's some limitation if running this application in QSPI flash in place.
If run it in QSPI flash, there's high latency when instruction cache miss. As two boards can't share
the cache line to reduce the numbers of cache miss times so it may cannot adapt to the cache miss latency under board to board mode.
So when running in QSPI flash with board to board connections, please reduce the TRANSFER_BAUDRATE value to avoid data loss issue.####

Note
~~~~~~~~~~~~~~
Because of the LPSPI signal pin allocating issue, you can not restart this demo/example by press the ResetButton(SW3):
The LPSPI1_SIN, LPSPI1_SOUT, LPSPI1_SCK and LPSPI0_SOUT pins is connected to PTA12, PTA13, PTA14 and PTA15 pins.
These pins is also used as BOO_CFG12, BOO_CFG13, BOO_CFG14 and BOO_CFG15 when the i.MX SoC reboot. These signals is
driven by LPSPI0 pins(PTA18, PTA19, PTA20 and PTA23), when user press ResetButton. This may cause M4 Core enter enter
incorrect boot modes and only re-powerup the board can let the M4 core exit such modes.

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
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
When the example runs successfully, you can see the similar information from the terminal as below.
If runs on one single board:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FreeRTOS LPSPI example start.
This example use one lpspi instance as master and another as slave on a single board.
Master and slave are both use interrupt way.
Please make sure you make the correct line connection. Basically, the connection is:
LPSPI_master -- LPSPI_slave
    CLK      --    CLK
    PCS      --    PCS
    SOUT     --    SIN
    SIN      --    SOUT

LPSPI master transfer completed successfully.
LPSPI slave transfer completed successfully.
LPSPI transfer all data matched !
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
If runs on two boards, slave should start first.
Master side:
~~~~~~~~~~~~
FreeRTOS LPSPI example start.
This example use two boards to connect with one as master and anohter as slave.
Master and slave are both use interrupt way.
Please make sure you make the correct line connection. Basically, the connection is:
LPSPI_master -- LPSPI_slave
    CLK      --    CLK
    PCS      --    PCS
    SOUT     --    SIN
    SIN      --    SOUT

LPSPI master transfer completed successfully.
LPSPI transfer all data matched !
~~~~~~~~~~~~~
SLAVE side:
~~~~~~~~~~~~~
FreeRTOS LPSPI example start.
This example use two boards to connect with one as master and anohter as slave.
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
