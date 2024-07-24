Overview
========
The flexspi_psram_polling_transfer example shows how to use flexspi driver with polling:

In this example, flexspi will send data and operate the external PSRAM connected with FLEXSPI. Some simple flash command will
be executed, such as Read Data and Write Data.
Example will write/read through the whole chip, through IP command and AHB command.

SDK version
===========
- Version: 2.16.000

Toolchain supported
===================
- GCC ARM Embedded  13.2.1

Hardware requirements
=====================
- Micro USB cable
- two MIMX8ULP-EVK/EVK9 board
- JLink Plus
- 5V power supply
- Personal Computer


Board settings
==============

Prepare the Demo
================
1.  Connect 5V power supply and JLink Plus to the board, switch SW10 to power on the board
2.  Connect a micro USB cable between the host PC and the J17 USB port on the target board.
3.  Open a serial terminal with the following settings:
    - 115200 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
4.  After power on board, stop the uboot and ensure the board will not autoboot to linux.
5.  Download the program to the target board.
6.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
The log below shows the output of the hello world demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FLEXSPI example started!
IP Command Read/Write data succeed at all address range !
AHB Command Read/Write data succeed at all address range !
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
