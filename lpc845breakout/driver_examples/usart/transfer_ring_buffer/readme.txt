Overview
========
The usart_interrupt_rb_transfer example shows how to use usart driver in interrupt way with
RX ring buffer enabled.

In this example, one uart instance connect to PC through, the board will send back all characters
that PC send to the board.

Note: The example echo every 8 characters.

Toolchain supported
===================
- GCC ARM Embedded  9.3.1
- MCUXpresso  11.3.0

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
The log below shows the output of the USART transfer using ring buffer driver example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
USART RX ring buffer example.
Send back received data.
Echo every 8 bytes.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
