Overview
========
The LPUART example for FreeRTOS demonstrates the possibility to use the LPUART driver in the RTOS.
The example uses single instance of LPUART IP and writes string into, then reads back chars.
After every 4B received, these are sent back on LPUART.

Toolchain supported
===================
- GCC ARM Embedded  10.2.1
- MCUXpresso  11.5.0

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
