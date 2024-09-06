Overview
========
The flexspi_psram_dma_transfer example shows how to use flexspi driver with dma:

In this example, flexspi will send data and operate the external PSRAM connected with FLEXSPI. Some simple flash command will
be executed, such as Read Data and Write Data.
Example will write/read through the whole chip, using DMA transfer way.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- MCUXpresso  11.10.0
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Micro USB cable
- RD-RW61X-BGA board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the MCU-Link USB port (J7) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows the output of the FlexSPI example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FLEXSPI example started!
DMA Command Read/Write data succeed at all address range!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
