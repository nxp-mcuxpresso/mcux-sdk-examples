Overview
========
This example demonstrate configuration and use of the USART module in interrupt-driven 
asynchronous mode on communication with a terminal emulator calling the USART 
transactional APIs. USART will echo back every character to terminal emulator, and send
back all received characters once users press [Enter] key.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.4.0

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
The log below shows the output of the USART terminal driver example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Usart functional API interrupt example.
Board receives characters then sends them out.
Please input characters and press the Enter key to finish input:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
