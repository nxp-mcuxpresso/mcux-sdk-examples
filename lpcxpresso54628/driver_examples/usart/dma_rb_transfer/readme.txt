Overview
========
The usart dma ring buffer example shows how to use usart driver with DMA driver used:

In this example, a ring buffer was implemented to receive the characters from PC side, 
to implemente the ring buffer, a descriptor was created and chain itself as the next descriptor,
then the DMA transfer will start a continuous transfer.

While data in the ring buffer reach 8 characters, routine will send them out using DMA mode.


Toolchain supported
===================
- MCUXpresso  11.4.0
- GCC ARM Embedded  10.2.1

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso54628 board
- Personal Computer

Board settings
==============
No special is needed.

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the LPC-Link USB port (J8) on the board.
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Reset the SoC and run the project.

Running the demo
================
When the demo runs successfully, the log would be seen on the terminal like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
USART DMA ring buffer example
Board will send back received data

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
