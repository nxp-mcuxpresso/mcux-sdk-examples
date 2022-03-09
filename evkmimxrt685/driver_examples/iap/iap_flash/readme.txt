Overview
========
The IAP flash project is a simple demonstration program of the SDK IAP driver.
It provides some flash operations with ROM API, such as automatically detect flash
configuration, erase sectors and program pages. All operations are followed by
flash memory check to ensure the data operated are successful.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT685 board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J5) on the board
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
The log below shows the output of the iap flash example in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
IAP flash example started!

***NOR Flash Initialization Success!***

***NOR Flash Erase Success!***

***NOR Flash All Pages Read/Write Success!***
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
