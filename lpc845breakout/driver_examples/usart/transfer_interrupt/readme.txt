Overview
========
The usart_interrupt example shows how to use usart transactional API in interrupt way:

In this example, one usart instance connect to PC through, the board will send back all 
characters that PC send to the board.

Note: The example echo every 8 characters, so input 8 characters every time.

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
No special settings are required.

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
The log below shows the output of the USART transfer using interrupt driver example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Usart interrupt transfer example.
Board receives 8 characters then sends them out.
Now please input:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
