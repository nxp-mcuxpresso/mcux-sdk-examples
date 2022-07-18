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
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Micro USB cable
- FRDM-KE15Z board
- Personal Computer

Board settings
==============
No special settings are required.

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
When the example runs successfully, the following message is displayed in the terminal. You can also send characters to the console back and they will be printed out onto console in a group of 4 characters.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~
FreeRTOS LPUART driver example!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~
