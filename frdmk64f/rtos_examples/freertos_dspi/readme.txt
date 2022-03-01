Overview
========
The freertos_dspi example shows how to use DSPI driver in FreeRTOS:

In this example , one dspi instance used as DSPI master with blocking and another dspi instance used as DSPI slave .

1. DSPI master sends/receives data using task blocking calls to/from DSPI slave. (DSPI Slave uses interrupt to receive/
send the data)


Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- FRDM-K64F board
- Micro USB cable
- Personal Computer

Board settings
==============
The dspi_interrupt example is configured to use UART1 with PTE0 and PTE1 pins. To make JLink serial 
device work, the jumpers on MAPS-DOCK board should be set as following:
- JP5: UART1 part jumpers connected

To observe the example result, please connect the pins of CN2 and CN13 on FRDM-K64F board as follows:

        SPI0 (master)             SPI1 (slave)
  PCS0: PTD0 (Pin-93, J2:6)  <--> PTD4 (Pin-97  J6:4)
  SCK:  PTD1 (Pin-94, J2:12) <--> PTD5 (Pin-98  J6:5)
  MOSI: PTD2 (Pin-95, J2:8)  <--> PTD7 (Pin-100 J6:7)
  MISO: PTD3 (Pin-96, J2:10) <--> PTD6 (Pin-99  J6:6)

Prepare the Demo
================
1. Connect a micro USB cable between the PC host and USB port (J26) on the FRDM-K64F board.
2. Open a serial terminal on PC for JLink serial device with these settings:
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
When the demo runs successfully, the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FreeRTOS DSPI example start.
This example use one dspi instance as master and another as slave on one board.
Master and slave are both use interrupt way.
Please make sure you make the correct line connection. Basically, the connection is:
DSPI_master -- DSPI_slave
   CLK      --    CLK
   PCS0     --    PCS0
   SOUT     --    SIN
   SIN      --    SOUT
DSPI master transfer completed successfully.

DSPI slave transfer completed successfully.

DSPI transfer all data matched!!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

