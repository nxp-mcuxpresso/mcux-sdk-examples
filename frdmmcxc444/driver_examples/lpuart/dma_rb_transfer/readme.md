Overview
========
The lpuart_dma Ring Buffer Example project is to demonstrate usage of the KSDK lpuart driver.
In the example, a ring buffer will be used with DMA enabled, data received will be stored in ring 
buffer first, while IDLE line was detected, routine will read out the received characters, and echo back
them to the terminal by using DMA mode. so, you can see the your pressed characters on the terminal.

And the ring buffer is achieved based on the DMA's source address modulo feature, the base address
should be aligned to a 0-modulo-boundary.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXC444 board
- Personal Computer

Board settings
==============
No special is needed.

Prepare the Demo
================
1.  Connect a type-c USB cable between the host PC and the MCU-Link USB port (J13) on the target board.
2.  Open a serial terminal with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Reset the SoC and run the project.

Running the demo
================
When the demo runs successfully, the log would be seen on the OpenSDA terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~
LPUART DMA ring buffer example.
Board will send back received characters:

~~~~~~~~~~~~~~~~~~~~~~~~~~~
