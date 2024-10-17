Overview
========
The uart_interrupt_ring_buffer example shows how to use uart driver in interrupt way with
RX ring buffer enabled:

In this example, one uart instance connect to PC through uart, the board will
send back all characters that PC send to the board.

Note: The example echo every 8 characters, so input 8 characters every time.



SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini USB cable
- USB to TTL converter
- FRDM-MCXC242 board
- Personal Computer

Board settings
==============
Connect the USB to TTL converter to FRDM-MCXC242 board.
FRDM-MCXC242 UART pins:
- TX of USB2COM to J1-5
- RX of USB2COM to J1-7
- GND of USB2COM to J2-14

Prepare the Demo
================
1.  Connect a mini USB cable between the PC host and the OpenSDA USB port on the board.
2.  Connect the USB to TTL converter to PC host.
3.  Open a serial terminal on PC for the USB to TTL converter with these settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Reset the SoC and run the project.

Running the demo
================
When the demo runs successfully, the log would be seen on the UART Terminal port which connected to the USB2COM like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
UART RX ring buffer example
Send back received data
Echo every 8 bytes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
