Overview
========
The flexio_uart_interrupt example shows how to use flexio uart driver in interrupt way:

In this example, a flexio simulated uart connect to PC through USB-Serial, the board will send back all characters
that PC send to the board. Note: two queued transfer in this example, so please input even number characters.

SDK version
===========
- Version: 2.15.0

Toolchain supported
===================
- GCC ARM Embedded  12.2

Hardware requirements
=====================
- Micro USB cable
- MIMX8ULP-EVK/EVK9 board
- J-Link Debug Probe
- 5V power supply
- Personal Computer

Board settings
==============
The example requires connecting the FLEXIO0 pins with the USB2COM pins
The connection should be set as follows:
- PTA8(J20,10) on base board, TX of USB2COM connected
- PTA9(J20,9) on base board, RX of USB2COM connected
- GND(J20,7)  on base board, Ground of USB2COM connected

#### Please note this application can't support running with Linux BSP! ####

#### Please note there's some limitation if running this application in QSPI flash in place.
If run it in QSPI flash, there's high latency when instruction cache miss. The FlexIO UART has
no FIFO so it has critical timing requirement that UART data must be read in time, otherwise
overflow may occur which causes data loss. So when running in QSPI flash, please don't
input more than 8 characters each time. ####

Prepare the Demo
================
1.  Connect 5V power supply and J-Link Debug Probe to the board, switch SW10 to power on the board.
2.  Connect a micro USB cable between the host PC and the J17 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the example

Running the demo
================
When the demo runs successfully, the log would be seen on the UART Terminal port which connected to the USB2COM like:

~~~~~~~~~~~~~~~~~~~~~
Flexio uart interrupt example
Board receives 8 characters then sends them out
Now please input:
~~~~~~~~~~~~~~~~~~~~~
