Overview
========
The flexspi_nor_polling_transfer example shows how to use flexspi driver with polling:

In this example, flexspi will send data and operate the external Nor flash connected with FLEXSPI. Some simple flash command will
be executed, such as Write Enable, Erase sector, Program page.
Example will first erase the sector and program a page into the flash, at last check if the data in flash is correct.

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Mini/micro USB cable
- EVK-MIMXRT1064 board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the example runs successfully, you can see the similar information from the terminal as below.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~

FLEXSPI example started!
Vendor ID: 0xef
Erasing Serial NOR over FlexSPI...
Erase data - successfully. 
Program data - successfully. 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Note: 
1, this example do erase/program/read operations in internal quad flash(W25Q32JV) and its image also loads to internal flash(W25Q32JV) for flash target.
2, internal flash pin connection like below:
DQS   <---> GPIO_SPI_B1_00
CS    <---> GPIO_SPI_B1_06
CLK   <---> GPIO_SPI_B0_08
IO0   <---> GPIO_SPI_B0_02
IO1   <---> GPIO_SPI_B1_03
IO2   <---> GPIO_SPI_B1_02
IO3   <---> GPIO_SPI_B0_10
RESET <---> GPIO_SPI_B0_13
GND2  <---> VSS
VCC   <---> NVCC_GPIO
GND   <---> VSS
