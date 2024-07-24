Overview
========
The flexio_uart_interrupt_ring_buffer example shows how to use flexio uart driver in interrupt way with
RX ring buffer enabled:

In this example, a flexio simulated uart connect to PC through USB-Serial, the board will
send back all characters that PC send to the board.

Note: The example echo every 8 characters, so input 8 characters every time.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- USB Type-C cable
- MCIMX93AUTO-EVK  board
- J-Link Debug Probe
- 12V~20V power supply
- Personal Computer

Board settings
==============
The example requires connecting the FLEXIO1 pins with the USB2COM pins
The connection should be set as follows:
- FLEXIO_RX(J16-5) on base board, TX of USB2COM connected
- FLEXIO_TX(J16-3) on base board, RX of USB2COM connected
- GND(J16-39)   on base board, Ground of USB2COM connected

#### Please note this application can't support running with Linux BSP! ####

Note
Please run the application in Low Power boot mode (without Linux BSP).
The IP module resource of the application is also used by Linux BSP.
Or, run with Single Boot mode by changing Linux BSP to avoid resource
conflict.
Tips: After you insert the USB2COM, a new USBCOM will be found. The function output in new USBCOM.

Prepare the Demo
================
1.  Connect 12V~20V power supply and J-Link Debug Probe to the board, switch SW2 to power on the board.
2.  Connect a USB Type-C cable between the host PC and the J26 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either re-power up your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
When the example runs successfully, you can see the similar information from the terminal as below.

~~~~~~~~~~~~~~~~~~~~~
FLEXIO UART RX ring buffer example
Send back received data
Echo every 8 bytes
~~~~~~~~~~~~~~~~~~~~~
