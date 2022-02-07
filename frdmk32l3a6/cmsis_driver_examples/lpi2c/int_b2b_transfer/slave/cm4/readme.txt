Overview
========
CMSIS-Driver defines generic peripheral driver interfaces for middleware making it reusable across a wide 
range of supported microcontroller devices. The API connects microcontroller peripherals with middleware 
that implements for example communication stacks, file systems, or graphic user interfaces. 
More information and usage method please refer to http://www.keil.com/pack/doc/cmsis/Driver/html/index.html.

The lpi2c_int_b2b_transfer_master example shows how to use CMSIS i2c driver as master to do board to board transfer 
with interrupt:

In this example, one lpi2c instance as master and another lpi2c instance on the other board as slave. Master sends a 
piece of data to slave, and receive a piece of data from slave. This example checks if the data received from 
slave is correct.

Toolchain supported
===================
- MCUXpresso  11.4.0
- GCC ARM Embedded  10.2.1

Hardware requirements
=====================
- Micro USB cable
- Two FRDM-K32L3A6 boards
- Personal Computer

Board settings
==============
To make lpi2c example work(M4), connections needed to be as follows:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MASTER_BOARD        CONNECTS TO         SLAVE_BOARD
Pin Name     Board Location     Pin Name     Board Location
LPI2C1_SCL   J1-10(PTB14)       LPI2C1_SCL   J1-10(PTB14)
LPI2C1_SDA   J2-6(PTB6)         LPI2C1_SDA   J2-6(PTB6)
GND          J2-14              GND          J2-14
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To make lpi2c example work(M0), connections needed to be as follows:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
MASTER_BOARD        CONNECTS TO         SLAVE_BOARD
Pin Name     Board Location     Pin Name     Board Location
LPI2C3_SCL   J3-15(PTB15)       LPI2C3_SCL   J3-15(PTB15)
LPI2C3_SDA   J3-13(PTB16)       LPI2C3_SDA   J3-13(PTB16)
GND          J2-14              GND          J2-14
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Prepare the Demo
================
1. Connect a micro USB cable between the PC host and the OpenSDA USB port on the board.
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
The following message shows in the terminal if the example runs successfully.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
CMSIS LPI2C board2board interrupt example -- Slave transfer.


End of LPI2C example .
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
