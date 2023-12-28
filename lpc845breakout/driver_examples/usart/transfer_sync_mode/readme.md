Overview
========
The usart_interrupt_sync_transfer example shows how to use usart API in synchronous mode:

In this example, one usart instance will be selected as master ,and another as slave. The master 
will send data to slave in polling way, and slave will receive data in nonblocking way.
After all data has been received by slave, info will be printed by debug console.

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
- LPC845 Breakout board
- Personal Computer

Board settings
==============
Please connect the USART1_CLK(P0_26) pin to USART2_CLK(P0_27) pin directly.
No need to connect other pins. The data pins are shared.

Prepare the demo
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port on the board.
2.  Open a serial terminal with the following settings:
    - 9600 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Either press the reset button on your board or launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows the output of the USART transfer using sync mode driver example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Usart synchronous transfer example.
Master USART will send data to slave on one board.
All data matched, data transfer success!

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
