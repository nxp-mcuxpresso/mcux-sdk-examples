Overview
========
This example is showing data send and receive via FreeRTOS adapted LPUART driver. Program initially send
string into serial terminal via virtual COM. After that, user may send some custom input and
application will return every 4 characters back to console. If delay from last user input exceed
10s (receive timeout) notification about exceeded timeout appear and application will finish.
Example need only single LPUART instance.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1

Hardware requirements
=====================
- Micro USB cable
- MCIMX7ULP-EVK board
- J-Link Debug Probe
- 5V power supply
- Personal Computer

Board settings
==============
No special settings are required.

#### Please note this application can't support running with Linux BSP! ####
If run it in QSPI flash, there's high latency when instruction cache miss. As the LPUART has small
FIFO so it has critical timing requirement that input UART data must be read in time, otherwise
overflow may occur which causes data loss. To run LPUART driver in QSPI flash, either limit the input
data rate or do synchronization for data exchange.

Prepare the Demo
================
1.  Connect 5V power supply and J-Link Debug Probe to the board, switch SW1 to power on the board.
2.  Connect a micro USB cable between the host PC and the J6 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  Download the program to the target board.
5.  Either press the reset button on your board or launch the debugger in your IDE to begin running the example.

Running the demo
================
When the example runs successfully, the following message is displayed in the terminal. You can also send characters to the console back and they will be printed out onto console in a group of 4 characters.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FreeRTOS LPUART driver example!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
