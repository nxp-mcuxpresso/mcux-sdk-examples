Overview
========
The usart_dma_low_power example shows how to use usart to wake up the system
in low power modes, and how to wake up for DMA only.
In this example, one usart instance connects to PC through usart, the board will
start receiving characters from PC and then go into low power mode. Each character
from PC will only wake up the DMA but the CPU keeps in low power mode. Once 8
characters were received, system will be woken up and echo the characters to PC.

Toolchain supported
===================
- GCC ARM Embedded  10.3.1
- MCUXpresso  11.6.0

Hardware requirements
=====================
- Micro USB cable
- EVK-MIMXRT595 board
- Personal Computer

Board settings
==============

Prepare the Demo
================
1.  Connect a micro USB cable between the PC host and the CMSIS DAP USB port (J40) on the board
2.  Open a serial terminal with the following settings:
    - 9600 baud rate
    - 8 data bits
    - No parity
    - One stop bit
    - No flow control
3.  Download the program to the target board.
4.  Launch the debugger in your IDE to begin running the demo.

Running the demo
================
When the demo runs successfully, the log would be seen on the terminal as follows (Note: please
don't input more than 16 bytes once as it might lead to UART FIFO overflow and result in failure):

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
USART DMA low power example
Send back received data
Echo every 8 characters (1st char 'q' leads to end).
------------------
Input any key to start.
Now enter deep sleep and wait for 8 characters.
12345678
------------------
Input any key to start.
Now enter deep sleep and wait for 8 characters.
asdfghjk
------------------
Input any key to start.
Now enter deep sleep and wait for 8 characters.
qwertyui
Done!
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
