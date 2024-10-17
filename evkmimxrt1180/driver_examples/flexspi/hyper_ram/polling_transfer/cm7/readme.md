Overview
========
The flexspi_hyper_ram_polling_transfer example shows how to use flexspi driver with polling:

In this example, flexspi will send data and operate the external Hyper RAM connected with FLEXSPI. Some simple flash command will
be executed, such as Read ID, Read Data and Write Data.
Example will first Write 256 bytes of data from the start of Hyper ram, then read the data back. at last check if the data in flash is correct.

SDK version
===========
- Version: 2.16.100

Toolchain supported
===================
- GCC ARM Embedded  13.2.1
- MCUXpresso  11.10.0

Hardware requirements
=====================
- Mini/micro USB cable
- MIMXRT1180-EVK board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a USB cable between the host PC and the OpenSDA USB port on the target board.
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
When the example runs successfully, you can see the similar information from the terminal as below.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FLEXSPI HyperRAM example started!
Vendor ID: 12
IP Command Read/Write data succeed at 0x0 - 0x80000 !
AHB Command Read/Write data succeed at 0x0 - 0x80000 !
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
