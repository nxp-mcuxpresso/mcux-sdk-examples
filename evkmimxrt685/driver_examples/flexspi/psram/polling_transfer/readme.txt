Overview
========
The flexspi_psram_polling_transfer example shows how to use flexspi driver with polling:

In this example, flexspi will send data and operate the external PSRAM connected with FLEXSPI. Some simple flash command will
be executed, such as Read Data and Write Data.
Example will write/read through the whole chip, through IP command and AHB command.

Toolchain supported
===================
- MCUXpresso  11.3.0
- GCC ARM Embedded  9.3.1

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT685 board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J40) on the board
2.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows the output of the FlexSPI example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FLEXSPI example started!
IP Command Read/Write data succeed at all address range !
AHB Command Read/Write data succeed at all address range !
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
