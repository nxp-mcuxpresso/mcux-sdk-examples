Overview
========
The flexio_uart_interrupt_ring_buffer example shows how to use flexio uart driver in interrupt way with
RX ring buffer enabled:

In this example, a flexio simulated uart connect to PC through USB-Serial, the board will
send back all characters that PC send to the board.

Note: The example echo every 8 characters, so input 8 characters every time.

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.3
- MCUXpresso  11.9.0

Hardware requirements
=====================
- Type-C USB cable
- FRDM-MCXA156 Board
- USB to Serial Converter
- Personal Computer

Board settings
==============
The flexio_uart_polling example is requires connecting the FLEXIO pins with the USB2COM pins
The connection should be set as following:
- J8-13(P0_16), RX of USB2COM connected
- J8-14(P0_17), TX of USB2COM connected
- J8-2, Ground of USB2COM connected

Prepare the Demo
================
1.  Connect a Type-C USB cable between the host PC and the FRDM board J21.
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
When the demo runs successfully, the log would be seen on the UART Terminal port which connected to the USB2COM like:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FLEXIO UART RX ring buffer example
Send back received data
Echo every 8 bytes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
