Overview
========
The flexio_spi_master_interrupt example shows how to use flexio spi master  driver in interrupt way:

In this example, a flexio simulated master connect to a flexio simulated spi slave .

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Mini/micro USB cable
- Two MIMXRT1060-EVKB boards
- Personal Computer

Board settings
==============
Remove the resistor R347,R348,R349,R351,R363,R364,R365,R366.

To make the example work, connections needed to be as follows:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        MASTER        connect to       SLAVE
Pin Name   Board Location     Pin Name    Board Location
MOSI       J33-5                MOSI      J33-5
MISO       J16-2                MISO      J16-2
SCK        J33-6                SCK       J33-6
PCS0       J16-1                PCS0      J16-1
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
FlexIO SPI interrupt example
Master Start...

Master runs successfully!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
