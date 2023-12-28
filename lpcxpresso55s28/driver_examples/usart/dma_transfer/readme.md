Overview
========
The usart_dma example shows how to use usart driver with DMA:

In this example, one usart instance connect to PC through usart, the board will
send back all characters that PC send to the board.

Note: The example echo every 8 characters, so input 8 characters every time.

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2
- MCUXpresso  11.8.0

Hardware requirements
=====================
- Micro USB cable
- LPCXpresso55S28 board
- Personal Computer

Board settings
==============
No special is needed.

Prepare the Demo
================
Note: MCUXpresso IDE project default debug console is semihost
1.  Connect a micro USB cable between the PC host and the LPC-Link USB port (P6) on the board.
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
USART DMA example
Send back received data
Echo every 8 characters
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
