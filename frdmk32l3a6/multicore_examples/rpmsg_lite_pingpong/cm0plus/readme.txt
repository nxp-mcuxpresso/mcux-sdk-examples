Overview
========
The Multicore RPMsg-Lite pingpong project is a simple demonstration program that uses the
MCUXpresso SDK software and the RPMsg-Lite library and shows how to implement the inter-core
communicaton between cores of the multicore system. The primary core releases the secondary core
from the reset and then the inter-core communication is established. Once the RPMsg is initialized
and endpoints are created the message exchange starts, incrementing a virtual counter that is part
of the message payload. The message pingpong finishes when the counter reaches the value of 100.
Then the RPMsg-Lite is deinitialized and the procedure of the data exchange is repeated again.

Shared memory usage
This multicore example uses the shared memory for data exchange. The shared memory region is
defined and the size can be adjustable in the linker file. The shared memory region start address
and the size have to be defined in linker file for each core equally. The shared memory start
address is then exported from the linker to the application.

Toolchain supported
===================
- MCUXpresso  11.3.0
- GCC ARM Embedded  9.3.1

Hardware requirements
=====================
- Mini/micro USB cable
- FRDM-K32L3A6 board
- Personal Computer

Board settings
==============
No special settings are required.

Prepare the Demo
================
1.  Connect the PC host and the board
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
The log below shows the output of the RPMsg-Lite pingpong demo in the terminal window:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

RPMsg demo starts

Primary core received a msg
Message: Size=4, DATA = 1
Primary core received a msg
Message: Size=4, DATA = 3
Primary core received a msg
Message: Size=4, DATA = 5
.
.
.
Primary core received a msg
Message: Size=4, DATA = 101
.
.
.
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
